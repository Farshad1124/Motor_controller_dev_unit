#ifndef FOC_APP
#define FOC_APP

#define DEF_POWER_SUPPLY 12.0f // default power supply voltage

#define DEF_PID_VEL_P 0.5f //!< default PID controller P value
#define DEF_PID_VEL_I 10.0f //!<  default PID controller I value
#define DEF_PID_VEL_D 0.0f //!<  default PID controller D value
#define DEF_PID_VEL_RAMP 1000.0f //!< default PID controller voltage ramp value
#define DEF_PID_VEL_LIMIT (DEF_POWER_SUPPLY) //!< default PID controller voltage limit

#define DEF_PID_CURR_P 3.0f // default PID controller P value
#define DEF_PID_CURR_I 300.0f // default PID controller I value
#define DEF_PID_CURR_D 0.0f // default PID controller D value
#define DEF_PID_CURR_RAMP 0.0f  // default PID controller voltage ramp value
#define DEF_PID_CURR_LIMIT (DEF_POWER_SUPPLY) // default PID controller voltage limit
#define DEF_CURR_BANDWIDTH 300.0f // current bandwidth

// default current limit values
#define DEF_CURRENT_LIM 2.0f // 2Amps current limit by default

// angle P params
#define DEF_P_ANGLE_P 20.0f // default P controller P value
#define DEF_VEL_LIM 20.0f // angle velocity limit default

// index search
#define DEF_INDEX_SEARCH_TARGET_VELOCITY 1.0f // default index search velocity

// align voltage
#define DEF_VOLTAGE_SENSOR_ALIGN 3.0f // default voltage for sensor and motor zero alignemt

// low pass filter velocity
#define DEF_VELOCITY_CUTOFF 200.0f



enum FOC_Motion_Control_Mode
{
    torque_control,
    velocity_control,
    position_control,
    position_openloop_control,
    position_nocascade,
    velocity_openloop_control
};

enum FOC_Torque_Control_Mode
{
    Torque_via_voltage              = 0x10, // torque control via voltage measure
    Torque_via_dc_current           = 0x11, // torque control via single point current
    Torque_via_idq                  = 0x12, // torque control via id, iq current values
    Torque_via_estimated_current    = 0x13, // torque control via motor characteristics model
};

enum FOC_Message_type
{
    Change_current_limit,
    Change_voltage_limit,
    Change_torque_control_type,
    Change_velocity_limit,
    Change_motion_control_type,
    Set_target_angle,
    Set_target_velocity,
    Set_target_torque,
    Change_target,
};

enum FOCModulationType {
    SinePWM             = 0x20,
    SpaceVectorPWM      = 0x21,
    Trapezoid_120       = 0x22,
    Trapezoid_150       = 0x23
};

enum Direction : int8_t {
    CW      = 1,  // clockwise
    CCW     = -1, // counter clockwise
    UNKNOWN = 0   // not yet known or invalid state
};


typedef struct 
{
    float source_voltage;
    float voltage_limit;
    float current_limit;
    float velocity_limit;
    float ADC_gain;
    enum FOC_Motion_Control_Mode motion_mode;
    enum FOC_Torque_Control_Mode torque_mode;
    float torque_limit;
    uint32_t pwm_frequency;
    enum Direction sensor_direction;
    enum FOCModulationType modulation;
    float current_P;
    float current_I;
    float current_D;
} FOC_control_config;

typedef struct
{
    uint8_t pole_pair_count;
    float inductance;
    float winding_resistance;
    float KV_rating;
} FOC_motor_config;

typedef struct
{
    enum FOC_Message_type type;
    float value;
    enum FOC_Motion_Control_Mode control_mode;
    enum FOC_Torque_Control_Mode torque_mode;
} FOC_message;


void FOC_Application_Initialize (FOC_control_config* control, FOC_motor_config* motor);
void FOC_Move(float target);
bool active_state_check (void);
#endif