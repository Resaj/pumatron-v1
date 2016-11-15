#ifndef PID_LAB_h
#define PID_LAB_h

//Variables PID
extern int position_lab;
extern int proportional_lab;
extern long integral_lab;
extern long integral_max_lab;
extern int derivative_lab;
extern int last_proportional_lab;
extern int control_value_lab;
extern int max_difference_lab;
extern float Kp_lab;
extern float Ki_lab;
extern float Kd_lab;

#endif


// Funciones de control
void get_PID_lab(void);
void set_motors_lab(void);
void loop_PID_lab(void);
