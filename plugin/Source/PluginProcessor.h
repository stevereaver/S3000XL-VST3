/*
  ==============================================================================

    Ensoniq SD-1 MAME VST Emulation
    Open Source GPLv2/v3
    https://www.sojusrecords.com

  ==============================================================================
*/

#pragma once

// Uncomment to enable the debug rack panel (file manager + rack visible simultaneously)
//#define SD1_DEBUG_RACK_PANEL

// ==============================================================================
// MANDATORY MAME MACROS - MUST BE DEFINED BEFORE ANY MAME INCLUDES!
// These ensure compatibility with the MAME core data types and architectures.
// ==============================================================================
#ifndef PTR64
#define PTR64 1
#define LSB_FIRST 1
#define NDEBUG 1
#define __STDC_LIMIT_MACROS 1
#define __STDC_FORMAT_MACROS 1
#define __STDC_CONSTANT_MACROS 1
#endif

#include <JuceHeader.h>
#include <fstream> 

// MAME Core Includes
#include "emu.h"
#include "mame.h"

// Threading & Synchronization
#include <thread>
#include <atomic>
#include <mutex>

//==============================================================================

class EnsoniqSD1AudioProcessor : public juce::AudioProcessor,
    public juce::AudioProcessorValueTreeState::Listener
{
public:
    
    // --- VFD DISPLAY & LED HARDWARE STATES ---
        static constexpr int VFD_SIZE = 80; // 2 rows x 40 characters
        
        // Stores the raw 14-segment bitmask for each character
        std::atomic<uint16_t> vfdSegments[VFD_SIZE];
        
        // Stores the 32-bit integer where each bit represents a specific panel LED
        std::atomic<uint32_t> ledStateMask{ 0 };

        // MAME callback function triggered whenever a hardware output changes
        static void mameOutputNotifier(const char *outname, s32 value, void *param);
        
        // API for the Editor / File Manager to read the hardware state safely
        juce::String getHardwareVfdText();
        bool isHardwareLedOn(int ledBitIndex);

        // Dynamic dictionary to translate hardware bitmasks back to text
        std::unordered_map<uint16_t, char> segmentToAscii;
        void buildVfdDictionary();
    
    // New atomic flag to signal that the MAME engine is fully initialized and clocks are valid
        std::atomic<bool> mameIsFullyBooted{ false };
    
    // --- SELF CHECK ---
        std::atomic<bool> isSelfCheckFailed{ false };
        juce::String selfCheckErrorMsg { "" };
        bool runSelfCheck();
    
    // --- ROM MANAGEMENT ---
        juce::String customRomPath { "" };
    
    // --- Last browsed ---
    juce::String lastBrowsedFolder;
    juce::String lastMediaFolder;
    juce::String lastRomFolder;
    juce::String myComputerPath;    // Browse Computer current directory
        
        // --- FOLDER BOOKMARKS (max 10, persisted in settings.xml) ---
        juce::StringArray bookmarkFolders;
        
        // --- FILE MANAGER STATE (survives Editor destroy/recreate) ---
        struct FileManagerState {
            bool visible = false;
            juce::String category;          // "INT (RAM)", "ROM0", "BOOKMARK:/path", etc.
            juce::String openedFilePath;    // full path of opened file (if external)
            bool viewingDiskBank = false;
            juce::String openedDiskBankName;
            int selectedRow = -1;           // contentList row (fallback only)
            int bankSelectedRow = -1;       // bankContentList row (fallback only)
            juce::String selectedName;      // actual item name in contentList (primary restore key)
            juce::String bankSelectedName;  // actual item name in bankContentList (primary restore key)
            int scrollPosition = 0;         // contentList top row
            int bankScrollPosition = 0;     // bankContentList top row
            juce::String activeBookmark;    // bookmark path active at save time (for song state)
            int viewBeforeBrowser = 0;      // panel view index to restore when closing file manager
            int fmWindowWidth = 1200;       // Dedicated width for File Manager
            int fmWindowHeight = 925;       // Dedicated height for File Manager
        };
        FileManagerState fileManagerState;
        std::atomic<bool> stateJustLoaded{ false };  // prevents Editor destructor from overwriting song state
        std::atomic<bool> isWarmBoot{ false }; // NEW INSTANCE OR LOAD STATE
        std::atomic<bool> requestFileManagerUIRefresh{ false }; // Notifies GUI to update File Manager after state load
        std::atomic<bool> showWelcomeMessage{ false };  // Flag for first-launch UX message
        void checkRomAndBootMame();
    
    // --- GLOBAL SETTINGS ---
        std::atomic<bool> requestGlobalSave{ false };
        void loadGlobalSettings();
    
    // --- COMPARE STATE MANAGEMENT ---
    void forceCompareOff();
    std::atomic<double> scheduledCompareResetTime{ -1.0 };

    // --- MAME STATE MANAGEMENT ---
    // Used to safely orchestrate loading/saving states between the UI and the MAME thread
    std::atomic<bool> requestMameSave{ false };
    std::atomic<bool> requestMameLoad{ false };
    std::atomic<bool> mameStateIsReady{ false };
    juce::WaitableEvent mameStateEvent{ false };
    
    // Countdown timer (in samples) to trigger a delayed MIDI Panic after a state load
    std::atomic<int> panicDelaySamples{ 0 };

    // --- MEDIA HANDLING (FLOPPY/CARTRIDGE/SYSEX) ---
    std::atomic<bool> requestFloppyLoad{ false };
    std::atomic<bool> requestCartLoad{ false };
    std::string pendingFloppyPath;
    std::string pendingCartPath;
    std::mutex mediaMutex;
    
    // --- MEDIA STATE TRACKING ---
        std::atomic<bool> isFloppyLoaded{ false };
        std::atomic<bool> isCartLoaded{ false };
        juce::String loadedFloppyName{ "" };
        juce::String loadedCartName{ "" };

        // --- S3000XL SCSI media (scsi:5=harddisk, scsi:4=cdrom; always populated -> hot-swap) ---
        std::atomic<bool> requestHddLoad{ false };
        std::atomic<bool> requestCdLoad{ false };
        std::string pendingHddPath;
        std::string pendingCdPath;
        std::atomic<bool> isHddLoaded{ false };
        std::atomic<bool> isCdLoaded{ false };
        juce::String loadedHddName{ "" };
        juce::String loadedCdName{ "" };

    // --- WINDOW SIZE PERSISTENCE ---
    // Stores the last window size set by the user to recall it upon project load
    int savedWindowWidth{ 0 };
    int savedWindowHeight{ 0 };

    // --- SYNCHRONIZATION ---
    // Throttle event used to prevent MAME from generating audio faster than the DAW consumes it
    juce::WaitableEvent mameThrottleEvent{ false };
    bool isMameRunningFlag() const { return isMameRunning.load(); }
    std::atomic<bool> isRomMissing{ false };

    
    // Flag to indicate if the zip file exists but contains invalid/missing ROMs
    std::atomic<bool> isRomInvalid{ false };

    std::atomic<bool> mameHasStarted{ false };
    std::atomic<double> initialSampleRate{ 0.0 };
    std::atomic<bool> sampleRateMismatch{ false };
    
    // Flag to indicate if the plugin is running as an AU in an unsupported host (e.g., FL Studio, Ableton)
    std::atomic<bool> isUnsupportedAUHost{ false };
    bool isMaschineHost = false;  // Set once in prepareToPlay, read-only in processBlock
    bool maschineInFastRender = false; // True once WAV RENDER FIX confirms fast render; resets on stop
    
    uint64_t getTotalRead() const { return totalRead.load(std::memory_order_acquire); }
    uint64_t getTotalWritten() const { return totalWritten.load(std::memory_order_acquire); }

    //==============================================================================
    EnsoniqSD1AudioProcessor();
    ~EnsoniqSD1AudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    
            // FULL SD-1 HARDWARE MATRIX DEFINITION
            struct SD1ButtonDef {
                juce::String paramID;
                juce::String paramName;
                const char* ioportTag;
                uint32_t ioportMask;
            };

            const std::vector<SD1ButtonDef> sd1Buttons = {
                // S3000XL front-panel matrix (C0..C7). Masks verbatim from
                // MAME akai/s3000.cpp INPUT_PORTS_START(s3000xl). Tags are root-absolute ":Cn".
                // Mode buttons (mask 0x01, LED-backed)
                { "btn_load",    "Load",         ":C0", 0x01 },
                { "btn_save",    "Save",         ":C1", 0x01 },
                { "btn_global",  "Global",       ":C2", 0x01 },
                { "btn_edit",    "Edit",         ":C3", 0x01 },
                { "btn_effects", "Effects",      ":C4", 0x01 },
                { "btn_sample",  "Sample",       ":C5", 0x01 },
                { "btn_multi",   "Multi",        ":C6", 0x01 },
                { "btn_single",  "Single",       ":C7", 0x01 },
                // Soft keys (mask 0x02)
                { "btn_f8",      "F8",           ":C0", 0x02 },
                { "btn_f7",      "F7",           ":C1", 0x02 },
                { "btn_f6",      "F6",           ":C2", 0x02 },
                { "btn_f5",      "F5",           ":C3", 0x02 },
                { "btn_f4",      "F4",           ":C4", 0x02 },
                { "btn_f3",      "F3",           ":C5", 0x02 },
                { "btn_f2",      "F2",           ":C6", 0x02 },
                { "btn_f1",      "F1",           ":C7", 0x02 },
                // Numeric / edit (mask 0x04)
                { "btn_key1",    "1 / W",        ":C0", 0x04 },
                { "btn_key2",    "2 / X",        ":C1", 0x04 },
                { "btn_mark",    "Mark",         ":C2", 0x04 },
                { "btn_key9",    "9 / S",        ":C3", 0x04 },
                { "btn_key8",    "8 / R",        ":C4", 0x04 },
                { "btn_key7",    "7 / Q",        ":C5", 0x04 },
                { "btn_name",    "Name",         ":C6", 0x04 },
                { "btn_key3",    "3 / Y",        ":C7", 0x04 },
                // Numeric / edit (mask 0x08)
                { "btn_key0",    "0 / Z",        ":C0", 0x08 },
                { "btn_minus",   "- / >",        ":C1", 0x08 },
                { "btn_jump",    "Jump",         ":C2", 0x08 },
                { "btn_key6",    "6 / V",        ":C3", 0x08 },
                { "btn_key5",    "5 / U",        ":C4", 0x08 },
                { "btn_key4",    "4 / T",        ":C5", 0x08 },
                { "btn_enter",   "Enter / Play", ":C6", 0x08 },
                { "btn_plus",    "+ / <",        ":C7", 0x08 },
                // Cursor arrows (mask 0x10)
                { "btn_right",   "Right",        ":C1", 0x10 },
                { "btn_up",      "Up",           ":C2", 0x10 },
                { "btn_down",    "Down",         ":C6", 0x10 },
                { "btn_left",    "Left",         ":C7", 0x10 },
                // DATA ENTRY dial (analog) - two GUI click-zones -> analog_field::set_value
                { "btn_dial_dec", "Data Entry -", ":DATAENTRY", 0x01 },
                { "btn_dial_inc", "Data Entry +", ":DATAENTRY", 0x02 },
            };

        std::vector<std::atomic<float>*> buttonParams;

        // --- DATA ENTRY dial (analog) state ---
        std::atomic<int> dialValue { 128 };      // 0..255 analog position
        bool dialIncPrev = false, dialDecPrev = false;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Core function to boot the headless MAME environment
    void runMameEngine();
    
    // Verifies the unzipped ROM files in the sd132 directory
    bool verifyRomFiles();
    // Extracts only the required .bin files from a user-provided zip
    bool extractRomsFromZip(const juce::File& zipFile);
    // Copies the required .bin files from a user-provided directory
    bool copyRomsFromFolder(const juce::File& sourceDir);
    // Stores the list of missing ROM files to be displayed on the UI
    juce::String missingFilesList;

    // Callback to push generated audio from MAME into our ring buffers
    void pushAudioFromMame(const int16_t* pcmBuffer, int numSamples);

    // OSD output stream path: 2-channel interleaved int16 (L, R)
    void pushAudioFromMameOSD(const int16_t* buffer, int numSamples);

    // ========================================================
    // MIDI INPUT HANDLING (JUCE -> MAME)
    // ========================================================
    void pushMidiByte(uint8_t data, double targetMameTime);
    void clearMidiBuffer();
    bool pollMidiData();
    int readMidiByte();
    
    // --- MIDI OUTPUT (from SD-1 DUART TX → JUCE MIDI out) ---
    static constexpr int MIDI_OUT_BUFFER_SIZE = 16384;
    uint8_t midiOutBuffer[MIDI_OUT_BUFFER_SIZE];
    std::atomic<int> midiOutWritePos{ 0 };
    std::atomic<int> midiOutReadPos{ 0 };
    void pushMidiOutByte(uint8_t data);
    
    // MIDI output message assembler state
    std::vector<uint8_t> midiOutMsg;
    uint8_t midiOutRunningStatus = 0;
    bool midiOutInSysEx = false;

    // Pointer to the running MAME engine instance
    running_machine* mameMachine = nullptr;

    // --- MOUSE EVENT INJECTION (JUCE -> MAME) ---
    void injectMouseMove(int x, int y);
    void injectMouseDown(int x, int y);
    void injectMouseUp(int x, int y);

    // Thread-safe containers for mouse coordinates and button states
    std::atomic<int> mouseX{ 0 };
    std::atomic<int> mouseY{ 0 };
    std::atomic<uint32_t> mouseButtons{ 0 };

    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Triggered by the DAW when an automation parameter changes
    void parameterChanged(const juce::String& parameterID, float newValue) override;

    // Dynamically adjustable buffer threshold for MAME processing
    std::atomic<int> mameBufferThreshold{ 1024 };
    std::atomic<int> hostBlockSize{ 1 };

    int getEffectiveBufferThreshold() const {
        return juce::jmax(mameBufferThreshold.load(std::memory_order_relaxed),
                          hostBlockSize.load(std::memory_order_relaxed));
    }

    // Dynamic offline buffer for sync
    std::atomic<int> maxOfflineBuffer{ 1024 };
                
        // --- RAM INJECTION BUFFERS ---
        juce::MemoryBlock pendingOsram;
        juce::MemoryBlock pendingSeqRam;
        juce::MemoryBlock pendingWaveRam;
        std::atomic<bool> pendingRamInjection{ false };
    
        // --- BANK INJECTION (60-program bank → osram, no CPU reset) ---
        juce::MemoryBlock pendingBankData;          // interleaved 31800 bytes
        std::atomic<bool> pendingBankInjection{ false };
        
        // --- STATE LOAD COMPARE RESET ---
        /*std::atomic<bool> needsCompareReset{ false };*/
        
        // --- MIDI INPUT SUPPRESS (during Write Single Preset) ---
        std::atomic<bool> suppressMidiInput{ false };
        
        // --- MIDI OPERATION CANCEL (set by onClose to abort pending timers) ---
        std::atomic<bool> midiOpCancelled{ false };
    
        // AU COLD BOOT HACK
        std::atomic<bool> needsBootPreRoll { false };
    
        // --- DYNAMIC PANEL LAYOUT SELECTION ---
        // 0 = Compact, 1 = Full, 2 = Panel, 3 = Tablet
        std::atomic<int> requestedViewIndex{ 0 };
        std::atomic<bool> requestViewChange{ false };

        // --- PIXEL PERFECT RENDERING ---
        // Stores the exact physical pixel dimensions of the current JUCE window.
        // MAME will strictly render at this 1:1 resolution to save CPU and maximize sharpness.
        std::atomic<int> windowWidth{ 1200 };
        std::atomic<int> windowHeight{ 539 };
        std::atomic<bool> requestRenderResize{ false };

        std::atomic<bool>& getFrameFlag() { return newFrameAvailable; }
        double getHostSampleRate() const { return hostSampleRate.load(); }

        // --- DOUBLE BUFFERED VIDEO RENDERING ---
        // Increased to 2560x2560 to safely fit the maximum allowed VST window size
        juce::Image cachedTexture{ juce::Image::ARGB, 2560, 2560, true, juce::SoftwareImageType() };

        juce::Image screenBuffers[2]{
            juce::Image(juce::Image::ARGB, 2560, 2560, true, juce::SoftwareImageType()),
            juce::Image(juce::Image::ARGB, 2560, 2560, true, juce::SoftwareImageType())
        };

    // Indicates which screen buffer (0 or 1) is fully rendered and ready to be drawn by the UI
    std::atomic<int> readyBufferIndex{ 0 };
    
    // PendingAUMidi
    std::vector<std::pair<juce::MidiMessage, int>> pendingAUMidi;
    
    // AnchorSet for AU
    std::atomic<bool> auAnchorSet{ false };
    
    // --- MACRO STATE ---
    std::atomic<bool> isSaveMacroActive{ false };
    int saveMacroHeldBank = -1;                 // GUI-side: currently held bank (0-9), -1 = none
    std::atomic<int> macroBankToHold{ -1 };      // GUI → audio: bank to electronically hold via set_button
    std::atomic<int> detectedBankMask{ 0 };       // audio → GUI: bitmask of pressed banks (bit i = bank i)
    
    void shutdownMame();
    void startKeepAliveThread();
    void stopKeepAliveThread();
    void runKeepAliveThread();
    
private:

        // Member variables to replace the problematic 'static' variables in processBlock.
        // This ensures each plugin instance has its own independent state.
        bool lastIsPlaying = false;
        bool localLastOffline = false;
        double lastAuMidiTime = 0.0;
        uint64_t captureReadPos = 0;
    
    bool extractLegacyMameState(const juce::String& base64State, juce::MemoryBlock& outOsram, juce::MemoryBlock& outSeqram);
    std::thread mameThread;
    std::thread keepAliveThread;
    std::atomic<bool> isMameRunning{ false };
    std::atomic<bool> keepAliveRunning{ false };
    std::atomic<int64_t> lastProcessBlockMs{ 0 };

    std::atomic<uint64_t> totalRead{ 0 };
    std::atomic<double> hostSampleRate{ 44100.0 };
                
    int getInternalHardwareLatencySamples() const {
        double sr = hostSampleRate.load(std::memory_order_relaxed);
                int base = static_cast<int>(0.0244 * sr);
                return base;
    }

    // Audio Ring Buffers (Generously sized to prevent underruns)
    static constexpr int RING_BUFFER_SIZE = 65536;

    // --- MAIN OUT BUFFERS ---
    float ringBufferL[RING_BUFFER_SIZE] = { 0.0f };
    float ringBufferR[RING_BUFFER_SIZE] = { 0.0f };

    // --- AUX OUT BUFFERS ---
    float ringBufferAuxL[RING_BUFFER_SIZE] = { 0.0f };
    float ringBufferAuxR[RING_BUFFER_SIZE] = { 0.0f };

    std::atomic<uint64_t> totalWritten{ 0 };

    // --- Timestamped MIDI ---
    std::atomic<bool> needAnchorSync{ true };
    std::atomic<bool> prepareWasCalled{ false }; // NEW: Prevents Logic AU double-reset
    std::atomic<double> anchorMameTime{ 0.0 };
    std::atomic<uint64_t> anchorDawSample{ 0 };

    // Double precision seconds
    struct TimestampedMidi {
        uint8_t data;
        double targetMameTime;
    };

    static constexpr int MIDI_BUFFER_SIZE = 524288;
    TimestampedMidi midiBuffer[MIDI_BUFFER_SIZE];

    std::atomic<int> midiWritePos{ 0 };
    std::atomic<int> midiReadPos{ 0 };

    std::atomic<bool> newFrameAvailable{ false };
    
    bool lastOfflineState = false;
    int64_t lastPlayheadPos = 0;
    
    juce::String instanceTempDir; // Unique sandbox directory for this plugin instance
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnsoniqSD1AudioProcessor)
};
