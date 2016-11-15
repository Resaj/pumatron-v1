#ifndef PID_h
#define PID_h

//Variables PID
extern int position;
extern int proportional;
extern long integral;
extern long integral_max;
extern int derivative;
extern int last_proportional;
extern int control_value;
extern int max_difference;
extern float Kp;
extern float Ki;
extern float Kd;

#endif


// Funciones de control
void get_PID(void);
void set_motors(void);
void loop_PID(void);
