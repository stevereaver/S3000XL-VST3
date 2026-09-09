#pragma once
// Auto-generated from s3000xl.lay (view 6000x1112) + DATA ENTRY dial zones.
// Panel button screen-rects (view coords) -> buttonParams[] index (set_value / dial analog).
static constexpr float MPC_VIEW_W = 6000.0f;
static constexpr float MPC_VIEW_H = 1112.0f;
struct MPCClickRect { float x, y, w, h; int paramIndex; };
static const MPCClickRect mpcClickMap[] = {
    { 1951.0f, 890.0f, 145.0f, 107.0f, 0 }, // btn_load
    { 1734.0f, 890.0f, 145.0f, 107.0f, 1 }, // btn_save
    { 1517.0f, 890.0f, 145.0f, 107.0f, 2 }, // btn_global
    { 1300.0f, 890.0f, 145.0f, 107.0f, 3 }, // btn_edit
    { 1951.0f, 690.0f, 145.0f, 107.0f, 4 }, // btn_effects
    { 1734.0f, 690.0f, 145.0f, 107.0f, 5 }, // btn_sample
    { 1517.0f, 690.0f, 145.0f, 107.0f, 6 }, // btn_multi
    { 1300.0f, 690.0f, 145.0f, 107.0f, 7 }, // btn_single
    { 3810.0f, 895.0f, 145.0f, 107.0f, 8 }, // btn_f8
    { 3595.0f, 895.0f, 145.0f, 107.0f, 9 }, // btn_f7
    { 3380.0f, 895.0f, 145.0f, 107.0f, 10 }, // btn_f6
    { 3165.0f, 895.0f, 145.0f, 107.0f, 11 }, // btn_f5
    { 2950.0f, 895.0f, 145.0f, 107.0f, 12 }, // btn_f4
    { 2735.0f, 895.0f, 145.0f, 107.0f, 13 }, // btn_f3
    { 2520.0f, 895.0f, 145.0f, 107.0f, 14 }, // btn_f2
    { 2305.0f, 895.0f, 145.0f, 107.0f, 15 }, // btn_f1
    { 4652.0f, 810.0f, 145.0f, 76.0f, 16 }, // btn_key1
    { 4860.0f, 810.0f, 145.0f, 76.0f, 17 }, // btn_key2
    { 5305.0f, 530.0f, 145.0f, 76.0f, 18 }, // btn_mark
    { 5068.0f, 530.0f, 145.0f, 76.0f, 19 }, // btn_key9
    { 4860.0f, 530.0f, 145.0f, 76.0f, 20 }, // btn_key8
    { 4652.0f, 530.0f, 145.0f, 76.0f, 21 }, // btn_key7
    { 5305.0f, 810.0f, 145.0f, 76.0f, 22 }, // btn_name
    { 5068.0f, 810.0f, 145.0f, 76.0f, 23 }, // btn_key3
    { 4652.0f, 950.0f, 145.0f, 76.0f, 24 }, // btn_key0
    { 5068.0f, 950.0f, 145.0f, 76.0f, 25 }, // btn_minus
    { 5305.0f, 670.0f, 145.0f, 76.0f, 26 }, // btn_jump
    { 5068.0f, 670.0f, 145.0f, 76.0f, 27 }, // btn_key6
    { 4860.0f, 670.0f, 145.0f, 76.0f, 28 }, // btn_key5
    { 4652.0f, 670.0f, 145.0f, 76.0f, 29 }, // btn_key4
    { 5305.0f, 950.0f, 145.0f, 76.0f, 30 }, // btn_enter
    { 4860.0f, 950.0f, 145.0f, 76.0f, 31 }, // btn_plus
    { 4376.9f, 692.4f, 154.9f, 331.7f, 32 }, // btn_right
    { 4210.2f, 692.4f, 204.6f, 175.0f, 33 }, // btn_up
    { 4210.2f, 845.9f, 204.6f, 175.0f, 34 }, // btn_down
    { 4100.6f, 692.4f, 154.9f, 331.7f, 35 }, // btn_left
    { 4095.0f, 25.0f, 215.0f, 530.0f, 36 }, // btn_dial_dec
    { 4310.0f, 25.0f, 215.0f, 530.0f, 37 }, // btn_dial_inc
};
static constexpr int MPC_CLICK_COUNT = (int)(sizeof(mpcClickMap)/sizeof(mpcClickMap[0]));
