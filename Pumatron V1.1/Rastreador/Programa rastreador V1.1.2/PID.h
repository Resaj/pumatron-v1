#ifndef PID_h
#define PID_h

//Variables PID
extern int sensors_average;
extern int sensors_sum;
extern int position;
extern int proportional;
extern long integral;
extern int derivative;
extern int last_proportional;
extern int control_value;
extern int max_difference;
extern float Kp;
extern float Ki;
extern float Kd;

#endif


// Funciones de control
void get_average(void);
void get_sum(void);
void get_position(void);
void get_proportional(void);
void get_integral(void);
void get_derivative(void);
void get_control(void);
void adjust_control(void);
void set_motors(void);
void loop_PID(void);
