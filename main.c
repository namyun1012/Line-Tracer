#include "msp.h"
#include "Clock.h"
#include "stdio.h"

#define LED_RED 1
#define LED_GREEN (LED_RED << 1)
#define LED_BLUE (LED_RED << 2)
/**
 * main.c
 */
void pwm_init34(uint16_t period, uint16_t duty3, uint16_t duty4)
{
    TIMER_A0->CCR[0] = period;
    TIMER_A0->EX0 = 0x0000;

    TIMER_A0->CCTL[3] = 0x0040;
    TIMER_A0->CCR[3] = duty3;
    TIMER_A0->CCTL[4] = 0x0040;
    TIMER_A0->CCR[4] = duty4;

    TIMER_A0->CTL = 0x02F0;

    P2->DIR |= 0xc0;
    P2->SEL0 |= 0xc0;
    P2->SEL1 &= ~0xc0;
}

void motor_init(void)
{
    P3->SEL0 &= ~0xc0;
    P3->SEL1 &= ~0xc0;
    P3->DIR |= 0xc0;
    P3->OUT &= ~0xc0;

    P5->SEL0 &= ~0x30;
    P5->SEL1 &= ~0x30;
    P5->DIR |= 0x30;
    P5->OUT &= ~0x30;

    P2->SEL0 &= ~0xc0;
    P2->SEL1 &= ~0xc0;
    P2->DIR |= 0xc0;
    P2->OUT &= ~0xc0;

    pwm_init34(7500, 0, 0);
}

void led_init()
{
    P2->SEL0 &= ~0x07;
    P2->SEL1 &= ~0x07;

    P2->DIR |= 0x07;
    P2->OUT &= ~0x07;
}

void turn_on_led(int color)
{
    P2->OUT &= ~0x07;
    P2->OUT |= color;
}

void turn_off_led()
{
    P2->OUT &= ~0x07;
}

void sensor_init()
{
    P5->SEL0 &= ~0x08;
    P5->SEL1 &= ~0x08;
    P5->DIR |= 0x08;
    P5->OUT &= ~0x08;

    P9->SEL0 &= ~0x04;
    P9->SEL1 &= ~0x04;
    P9->DIR |= 0x04;
    P9->OUT &= ~0x04;

    P7->SEL0 &= ~0xFF;
    P7->SEL1 &= ~0xFF;
    P7->DIR &= ~0xFF;
}

void move(uint16_t leftDuty, uint16_t rightDuty)
{
    P3->OUT |= 0xc0;
    TIMER_A0->CCR[3] = leftDuty;
    TIMER_A0->CCR[4] = rightDuty;
}

void left_forward()
{
    P5->OUT &= ~0x10;
}

void left_backward()
{
    P5->OUT |= 0x10;
}

void right_forward()
{
    P5->OUT &= ~0x20;
}

void right_backward()
{
    P5->OUT |= 0x20;
}

uint16_t first_left;
uint16_t first_right;

uint16_t period_left;
uint16_t period_right;

uint32_t left_count;
void (*TimerA2Task)(void);

void TA2_0_IRQHandler(void)
{
    TIMER_A2->CCTL[0] &= ~0x0001;
    (*TimerA2Task)();
}

void TA3_0_IRQHandler(void)
{
    TIMER_A3->CCTL[0] &= ~0x0001;
    period_right = TIMER_A3->CCR[0] - first_right;
    first_right = TIMER_A3->CCR[0];
}

void TA3_N_IRQHandler(void)
{
    TIMER_A3->CCTL[1] &= ~0x0001;
    period_left = TIMER_A3->CCR[1] - first_left;
    first_left = TIMER_A3->CCR[1];
    left_count++;
}

uint32_t get_left_rpm()
{
    return 2000000 / period_left;
}

void (*TimerA2Task)(void);

void TimerA2_Init(void (*task)(void), uint16_t period)
{
    TimerA2Task = task;
    TIMER_A2->CTL = 0x0280;
    TIMER_A2->CCTL[0] = 0x0010;
    TIMER_A2->CCR[0] = (period - 1);
    TIMER_A2->EX0 = 0x0005;
    NVIC->IP[3] = (NVIC->IP[3] & 0xFFFFFF00) | 0x00000040;
    NVIC->ISER[0] = 0x00001000;
    TIMER_A2->CTL |= 0x0014;

}

void timer_A3_capture_Init()
{
    P10->SEL0 |= 0x30;
    P10->SEL1 &= ~0x30;
    P10->DIR &= ~0x30;

    TIMER_A3->CTL &= ~0x0030;
    TIMER_A3->CTL = 0x0200;

    TIMER_A3->CCTL[0] = 0x4910;
    TIMER_A3->CCTL[1] = 0x4910;
    TIMER_A3->EX0 &= ~0x0007;

    NVIC->IP[3] = (NVIC->IP[3] & 0x0000FFFF) | 0x40400000;
    NVIC->ISER[0] = 0x0000C000;
    TIMER_A3->CTL |= 0x0024;

}

/*
 * Systick Function
 */
void systick_init(void)
{
    SysTick->LOAD = 0x00FFFFFF;
    SysTick->CTRL = 0x00000005;
}

void systick_wait1ms()
{
    SysTick->LOAD = (48000000 / 1000);
    SysTick->VAL = 0;
    while ((SysTick->CTRL & 0x00010000) == 0)
    {
    };
}

void systick_wait1s()
{
    int i;
    int count = 1000;

    for (i = 0; i < count; i++)
    {
        systick_wait1ms();
    }
}

/*
 * led
 *
 */

#define LED_RED 1
#define LED_GREEN (LED_RED << 1)
#define LED_BLUE (LED_RED << 2)

void switch_init()
{
    P1->SEL0 &= ~0x12;
    P1->SEL1 &= ~0x12;

    P1->DIR &= ~0x12;

    P1->REN |= 0x12;
    P1->OUT |= 0x12;
}
void main(void)
{
    Clock_Init48MHz();
    led_init();
    sensor_init();
    motor_init();
    systick_init();
    timer_A3_capture_Init();
    led_init();
    switch_init();

    int old_speed = 600;
    int speed = 800;
    int rotate = 385; // 400
    int stop = 0;
    // right 부터 시작
    int sensor_1 = 0;
    int sensor_2 = 0;
    int sensor_5 = 0;
    int sensor_3 = 0;
    int sensor_4 = 0;
    int sensor_6 = 0;
    int sensor_7 = 0;
    int sensor_8 = 0;

    int phase_1 = 0;
    int phase_2 = 0;
    int phase_3 = 0;
    int phase_4 = 0;
    int phase_5 = 0;
    int phase_6 = 0;
    int phase_7 = 0;
    int phase_8 = 0;

    printf("program started!");
    while (1)
    {
        // Turn on IR LED;
        P5->OUT |= 0x08;
        P9->OUT |= 0x04;

        // Capacior charge
        P7->DIR = 0xff;
        P7->OUT = 0xff;
        Clock_Delay1us(10);

        // full charge 까지 기다림
        P7->DIR = 0x00;

        Clock_Delay1us(1000); //변경 필요?

        // Read Sensor
        // white : 0, black : 1
        sensor_1 = P7->IN & 0b001;
        sensor_2 = P7->IN & 0b010;
        // sensor 5 가운데
        sensor_3 = P7->IN & 0b100; //sensor 3 오른쪽
        sensor_4 = P7->IN & 0b1000;
        sensor_5 = P7->IN & 0b10000;
        sensor_6 = P7->IN & 0b100000; //
        sensor_7 = P7->IN & 0b1000000;
        sensor_8 = P7->IN & 0b10000000;

        // 각 phase 구현
        //phase_1
        // 1,4,5,8
        if (!phase_1 && sensor_1 && !sensor_2 && sensor_4 && sensor_5
                && !sensor_7 && sensor_8)
        {
            turn_on_led(LED_RED);
            printf("phase1 executed");
            move(0, 0);

            systick_wait1s();
            systick_wait1s();
            systick_wait1s();

            phase_1 = 1;

            turn_off_led();

            P5->OUT &= ~0x08;
            P9->OUT &= ~0x04;

            Clock_Delay1ms(10);

            continue;
        }
        /*
         * phase_2
         * 2,4,5,7
         */
        if (phase_1 && !phase_2 && !sensor_1 && sensor_2 && !sensor_3
                && sensor_4 && sensor_5 && !sensor_6 && sensor_7 && !sensor_8)
        {
            turn_on_led(LED_GREEN);
            printf("phase2 executed");
            int repet = 0;

            while (repet < 4)
            {
                if (repet % 2 == 0)
                {
                    left_forward();
                    right_forward();
                    move(speed - 100, speed - 100);
                }
                else
                {
                    left_backward();
                    right_backward();
                    move(speed - 100, speed - 100);
                }

                repet++;
                systick_wait1s();
                systick_wait1s();

            }

            P5->OUT &= ~0x08;
            P9->OUT &= ~0x04;

            Clock_Delay1ms(10);
            phase_2 = 1;

            turn_off_led();
            continue;
        }

        // 감속 phase_3
        // 1,4,5

        if (phase_2 && !phase_4 && sensor_1 && !sensor_2 && !sensor_3
                && sensor_4 && sensor_5 && !sensor_6 && !sensor_7 && !sensor_8)
        {
            if(!phase_3) left_count = 0;

            turn_on_led(LED_BLUE);
            printf("phase3 executed");
            left_forward();
            right_forward();
            move(speed - 200 - (left_count / 2), speed - 200 - (left_count / 2));

            P5->OUT &= ~0x08;
            P9->OUT &= ~0x04;
            Clock_Delay1ms(10);

            if(left_count % 10 == 0) {
                left_backward();
                right_forward();
                move(speed - 200 - (left_count /2), speed - 200 - (left_count / 2));
            }


            phase_3 = 1;
            continue;
        }

        // 가속 phase_4
        // 4,5,8
        if (phase_3 && !phase_5 && sensor_4 && sensor_5 && !sensor_6
                && !sensor_7 && sensor_8)
        {
            left_count = 0;
            turn_on_led(LED_RED);
            printf("phase4 executed");
            left_forward();
            right_forward();
            move(speed + 400, speed + 400);

            // for continue;

            P5->OUT &= ~0x08;
            P9->OUT &= ~0x04;

            Clock_Delay1ms(10);
            phase_4 = 1;
            continue;
        }
        /*
         * phase_4 add
         * phase_5 Rotate and forward
         * 4,5,7
         */

        if (phase_4 && !phase_5 && !sensor_1 && !sensor_2 && !sensor_3 && sensor_4
                && sensor_5 && !sensor_6 && sensor_7 && !sensor_8)
        {
            turn_on_led(LED_GREEN);
            printf("phase5 executed");

            // left
            left_count = 0;
            while (left_count < 2 * rotate - 100) // rotate 의 수를 줄일 수 있다.
            {
                left_backward();
                right_forward();
                move(speed * 2, speed * 2);
            }
            /*
             * move slightly
             */
            right_backward();
            left_forward();
            move(speed, speed);

            left_forward();
            right_forward();
            move(speed, speed);
            systick_wait1s();

            // right
            left_count = 0;
            while (left_count < 2 * rotate)
            {
                right_backward();
                left_forward();
                move(speed * 2, speed * 2);
            }

            // left
            left_count = 0;
            while (left_count < 2 * rotate)
            {
                right_forward();
                left_backward();
                move(speed * 2, speed * 2);
            }

            phase_5 = 1;

            P5->OUT &= ~0x08;
            P9->OUT &= ~0x04;

            Clock_Delay1ms(10);
            turn_off_led();

            continue;
        }

        /*

         * Phase 6
         right -> left sensor 2,4,5
         */

        if (phase_5 && !phase_6 && phase_5 && !sensor_1 && sensor_2 && !sensor_3
                && sensor_4 && sensor_5 && !sensor_6)
        {
            turn_on_led(LED_BLUE);

            printf("phase6 executed");

            left_forward();
            right_forward();
            move(old_speed, old_speed); //안될시 phase_6 전부 old_speed로 바꿀것

            systick_wait1s();
            systick_wait1s();
            systick_wait1s();

            left_count = 0;
            while (left_count < rotate)
            {
                left_backward();
                right_forward();
                move(speed * 2, speed * 2);
            }

            left_forward();
            right_forward();
            move(old_speed, old_speed);

            systick_wait1s();
            systick_wait1s();
            systick_wait1s();

            left_count = 0;
            while (left_count < rotate - 50)
            {
                right_backward();
                left_forward();
                move(speed * 2, speed * 2);
            }

            phase_6 = 1;
            continue;
        }

        /*
         * END
         */
        if (phase_6 && !sensor_1 && sensor_3 && sensor_4 && sensor_5 && sensor_6
                && !sensor_8)
        {
            move(0, 0);
            break;
        }
        // 직진
        if (sensor_4 && sensor_5)
        {
            left_forward();
            right_forward();
            move(speed, speed);
        }

        // 우회전
        else if (sensor_1 || sensor_2 || sensor_3 && !sensor_5)
        { // turn left if,else 특성상 조건을 빡빡하게
            left_forward();
            right_backward();
            move(speed, speed);
        }

        // 좌회전
        else if (sensor_6 || sensor_7 || sensor_8 && !sensor_4)
        { // turn right
            left_backward();
            right_forward();
            move(speed, speed);
        }

        // 임시용
        else if (sensor_4)
        {
            left_forward();
            right_backward();
            move(speed, speed);
        }

        else if (sensor_5)
        {
            left_backward();
            right_forward();
            move(speed, speed);
        }
        else
        {
            left_backward();
            right_backward();
            move(speed - 200, speed - 200);
        }

        P5->OUT &= ~0x08;
        P9->OUT &= ~0x04;
        turn_off_led();
        Clock_Delay1ms(10);
    }

}

