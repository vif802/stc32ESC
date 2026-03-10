
#include "zf_tim.h"
#include "zf_gpio.h"
#include "bldc_config.h"
#include "pwm.h"
#include "pwm_input.h"
#include "motor.h"
#include "battery.h"
#include "pit_timer.h"

#define LED_PIN P43

uint32 stime;

//-------------------------------------------------------------------------------------------------------------------
//  @brief      LED锟狡癸拷锟斤拷锟?//  @param      void                        
//  @return     void          
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void led_control(void)
{
    // LED状态锟斤拷示
    if(battery_low_voltage)
    {
        // 锟斤拷氐锟窖癸拷锟斤拷停锟絃ED锟斤拷锟斤拷 锟斤拷锟斤拷
        if(0 == (stime%100))
        {
            LED_PIN = 0;
        }
        else if(10 == (stime%100))
        {
            LED_PIN = 1;
        }
    }
    else if(motor.restart_delay)
    {
        // 锟斤拷锟节讹拷转锟斤拷锟斤拷停止 LED锟斤拷锟斤拷
        if(0 == (stime%5))
        {
            LED_PIN = !LED_PIN;
        }
    }
    else if(0 == motor.run_flag)
    {
        // 停止 LED锟斤拷锟斤拷
        if(0 == (stime%100))
        {
            LED_PIN = !LED_PIN;
        }
    }
    else if(1 == motor.run_flag)
    {
        // 锟斤拷锟斤拷位锟阶讹拷 LED锟斤拷锟斤拷锟斤拷
        if(0 == (stime%50))
        {
            LED_PIN = !LED_PIN;
        }
    }
    else if(2 == motor.run_flag)
    {
        // 锟斤拷锟斤拷锟斤拷锟劫阶讹拷 LED锟饺筹拷
        if(0 == (stime%10))
        {
            LED_PIN = !LED_PIN;
        }
    }
    else if(3 == motor.run_flag)
    {
        // 锟斤拷锟斤拷锟斤拷锟斤拷锟叫伙拷锟阶讹拷 LED锟饺筹拷
        LED_PIN = 0;
    }
    else if(4 == motor.run_flag)
    {
        // 锟斤拷锟斤拷锟斤拷锟斤拷 LED锟斤拷锟斤拷
        LED_PIN = 0;
    }
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      锟斤拷时锟斤拷1锟叫讹拷
//  @param      void                        
//  @return     void          
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void TM1_Isr() interrupt 3
{
    stime++;
    
    battery_voltage_get();
    
    led_control();
    
    if(motor.restart_delay)
    {
        // 锟斤拷时锟斤拷锟斤拷时锟斤拷锟斤拷锟
        motor.restart_delay--;
    }
    else
    {
        if(0 == battery_low_voltage)
        {
            if(motor.duty && (0 == motor.run_flag))
            {
                motor_start();
            }
            else if((0 == motor.duty) && (0 != motor.run_flag))
            {
                // 锟斤拷要锟斤拷锟斤拷锟斤拷诮锟斤拷锟斤拷停锟斤拷锟斤拷锟斤拷锟阶讹拷停锟斤拷锟斤拷锟斤拷锟街癸拷
                motor_stop();
            }
        }
        else
        {
            // 锟斤拷锟斤拷锟斤拷锟斤拷停止锟斤拷锟斤拷
            motor_stop();
        }
    }
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      锟斤拷锟节讹拷时锟斤拷锟斤拷始锟斤拷
//  @param      void                        
//  @return     void          
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void pit_timer_init(void)
{
    stime = 0;
    pit_timer_ms(TIM_1, 10);
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      LED锟斤拷始锟斤拷
//  @param      void                        
//  @return     void          
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void led_init(void)
{
    gpio_mode(P4_3, GPO_PP);
    LED_PIN = 0;
}
