

#include <math.h>
#include "zf_pwm.h"
#include "zf_gpio.h"
#include "zf_delay.h"
#include "zf_exti.h"
#include "zf_tim.h"
#include "comparator.h"
#include "bldc_config.h"
#include "pwm.h"
#include "battery.h"
#include "motor.h"


motor_struct motor;
uint8 timer4_isr_flag;

//-------------------------------------------------------------------------------------------------------------------
//  @brief      鐢垫満step鍔犱竴
//  @param      void 		    
//  @return     				
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void motor_next_step(void)
{
    motor.step++;
    while(6 <= motor.step)
    {
        motor.step -= 6;
    }
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      璁＄畻鍔犻€熸洸绾

//  @param      step            褰撳墠姝ユ暟
//  @param      total_steps     鎬绘鏁

//  @since      v1.0
//  @note       鏀寔涓夌鍔犻€熸洸绾? 0-绾挎€?1-鎸囨暟 2-S鏇茬嚎
//-------------------------------------------------------------------------------------------------------------------
uint16 calculate_acceleration_curve(uint16 step, uint16 total_steps)
{
    float ratio = (float)step / (float)total_steps;
    float delay;

    switch(START_OPEN_LOOP_ACCEL_CURVE)
    {
        case 1:  // 鎸囨暟鍔犻€?(鍏堟參鍚庡揩)
            delay = START_OPEN_LOOP_MIN_DELAY + (START_OPEN_LOOP_MAX_DELAY - START_OPEN_LOOP_MIN_DELAY) * (1.0f - ratio * ratio);
            break;

        case 2:  // S鏇茬嚎鍔犻€?(Sigmoid鍑芥暟锛岃捣姝ュ拰缁撳熬閮借緝骞虫粦)
            // Sigmoid: 1/(1+e^(-x)), x鑼冨洿-5鍒?瀵瑰簲0鍒

            delay = START_OPEN_LOOP_MIN_DELAY + (START_OPEN_LOOP_MAX_DELAY - START_OPEN_LOOP_MIN_DELAY) / (1.0f + exp((ratio - 0.5f) * 10));
            break;

        case 0:  // 绾挎€у姞閫

        default: // 榛樿浣跨敤绾挎€у姞閫

            delay = START_OPEN_LOOP_MAX_DELAY - (START_OPEN_LOOP_MAX_DELAY - START_OPEN_LOOP_MIN_DELAY) * ratio;
            break;
    }

    if(delay < START_OPEN_LOOP_MIN_DELAY) delay = START_OPEN_LOOP_MIN_DELAY;
    if(delay > START_OPEN_LOOP_MAX_DELAY) delay = START_OPEN_LOOP_MAX_DELAY;

    return (uint16)delay;
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      鍗犵┖姣旀枩鍧″鍔?//  @param      void
//  @return     void
//  @since      v1.0
//  @note       鍦ㄥ紑鐜樁娈甸€愭澧炲姞鍗犵┖姣旓紝閬垮厤鐢垫祦鍐插嚮
//-------------------------------------------------------------------------------------------------------------------
void duty_ramp_up(void)
{
    if(motor.duty_register < motor.target_duty)
    {
        motor.duty_ramp_timer++;
        if(motor.duty_ramp_timer >= START_DUTY_RAMP_PERIOD)
        {
            motor.duty_ramp_timer = 0;
            motor.duty_register += START_DUTY_RAMP_STEP;
            if(motor.duty_register > motor.target_duty)
            {
                motor.duty_register = motor.target_duty;
            }
            pwm_center_duty_update(motor.duty_register);
        }
    }
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      鐢垫満鎹㈢浉鍑芥暟
//  @param      void 		    
//  @return     				
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void motor_commutation(void)
{
    switch(motor.step)
    {
        case 0:
        {
            pwm_a_bn_output();
            // 鍦ㄨ繃娓￠樁娈?3)鍜岄棴鐜樁娈?4)閮介渶瑕佽缃瘮杈冨櫒
            if(3 == motor.run_flag || 4 == motor.run_flag)
            {
                comparator_falling();
            }

        }break;
        case 1:
        {
            pwm_a_cn_output();
            if(3 == motor.run_flag || 4 == motor.run_flag)
            {
                comparator_rising();
            }

        }break;
        case 2:
        {
            pwm_b_cn_output();
            if(3 == motor.run_flag || 4 == motor.run_flag)
            {
                comparator_falling();
            }

        }break;
        case 3:
        {
            pwm_b_an_output();
            if(3 == motor.run_flag || 4 == motor.run_flag)
            {
                comparator_rising();
            }

        }break;
        case 4:
        {
            pwm_c_an_output();
            if(3 == motor.run_flag || 4 == motor.run_flag)
            {
                comparator_falling();
            }

        }break;
        case 5:
        {
            pwm_c_bn_output();
            if(3 == motor.run_flag || 4 == motor.run_flag)
            {
                comparator_rising();
            }

        }break;
        default:break;
    }
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      鏌ユ壘鏈€澶?鏈€灏忓€硷紙PWM涓柇鍐呬娇鐢級
//  @param      *dat            闇€瑕佹煡鎵剧殑鏁版嵁鐩殑鍦板潃
//  @param      length 	        鏁版嵁闀垮害
//  @param      *max 	        淇濆瓨鏈€澶у€肩殑鍦板潃
//  @param      *min 	        淇濆瓨鏈€灏忓€肩殑鍦板潃
//  @return     				
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void motor_max_min_pwm(uint16 *dat, uint8 length, uint16 *max, uint16 *min)
{
    uint8 i;
    uint16 temp;

    *min = 0xffff;
    *max = 0;
    for(i=0; i<length; i++)
    {
        temp = dat[i];
        if(temp > *max)
        {
            *max = temp;
        }
        if(temp < *min)
        {
            *min = temp;
        }
    }
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      鏌ユ壘鏈€澶?鏈€灏忓€硷紙姣旇緝鍣ㄤ腑鏂唴浣跨敤锛?//  @param      *dat            闇€瑕佹煡鎵剧殑鏁版嵁鐩殑鍦板潃
//  @param      length 	        鏁版嵁闀垮害
//  @param      *max 	        淇濆瓨鏈€澶у€肩殑鍦板潃
//  @param      *min 	        淇濆瓨鏈€灏忓€肩殑鍦板潃	    
//  @return     				
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void motor_max_min_comparator(uint16 *dat, uint8 length, uint16 *max, uint16 *min)
{
    uint8 i;
    uint16 temp;

    *min = 0xffff;
    *max = 0;
    for(i=0; i<length; i++)
    {
        temp = dat[i];
        if(temp > *max)
        {
            *max = temp;
        }
        if(temp < *min)
        {
            *min = temp;
        }
    }
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      瀹氭椂鍣?閲嶆柊閰嶇疆
//  @param      void
//  @return     uint16          杩斿洖褰撳墠璁℃椂鍣ㄧ殑鏃堕棿
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
uint16 tim4_reconfig(void)
{
    uint16 temp;
    // 鑾峰彇鎹㈢浉鏃堕棿
    T4T3M &= ~0x80; // 鍋滄瀹氭椂鍣

    if(timer4_isr_flag)
    {
        timer4_isr_flag = 0;
        temp = 65535;
    }
    else
    {
        temp = T4H;
        temp = (temp << 8) | T4L;
    }
    T4L = 0;
    T4H = 0;
    T4T3M |= 0x80;  // 寮€鍚畾鏃跺櫒
    
    return temp;
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      瀹氭椂鍣?閲嶆柊閰嶇疆
//  @param      us              涓嬩竴娆¤Е鍙戝畾鏃跺櫒涓柇鐨勬椂闂?鍗曚綅寰
//  @return     void          
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void tim3_reconfig(uint32 us)
{
    // 涓婚30M 瀹氭椂鍣?2T妯″紡  瀹氭椂鍣ㄥ€?0xffff - us/(1/(30/12))  鍗冲畾鏃跺櫒鍊?0xffff - us*5/2
    // 鍚庣画灏嗕富棰戞彁鍗囦负33.1776MHZ锛岀敱浜庣浉杈冧簬30M宸窛骞朵笉鏄庢樉锛屽洜姝よ繖閲屼笉鍐嶉噸鏂拌绠楄浆鎹㈢郴鏁?    us = us*5/2;
    if(0xffff < us)
    {
        us = 0xffff;
    }
    
    us = 0xffff - us;
    
    T4T3M &= ~0x08;         // 鍋滄瀹氭椂鍣?    T3L = (uint8)us;
    T3H = (uint8)(us >> 8); 	
    T4T3M |= 0x08;          // 寮€鍚畾鏃跺櫒
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      瀹氭椂鍣?閲嶆柊閰嶇疆
//  @param      time            涓嬩竴娆¤Е鍙戝畾鏃跺櫒涓柇鐨勬椂闂?//  @return     void          
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void timer0_reconfig(uint16 time)
{
    TR0 = 0; 
    TL0 = (uint8)time;
    TH0 = (uint8)(time >> 8); 	
    TR0 = 1; 
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      瀹氭椂鍣?涓柇鍑芥暟
//  @param      void            
//  @return     void          
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void TM4_Isr(void) interrupt 20
{
    T4T3M &= ~0x80; // 鍏抽棴瀹氭椂鍣?	TIM4_CLEAR_FLAG;
    
    // 鎹㈢浉瓒呮椂
    timer4_isr_flag = 1;
    
    // 鍦ㄥ紑鐜?2)銆佽繃娓?3)鎴栭棴鐜?4)闃舵妫€娴嬪埌鎹㈢浉瓒呮椂锛屽垽瀹氫负鍫佃浆
    if((2 == motor.run_flag) || (3 == motor.run_flag) || (4 == motor.run_flag))
    {
        // 姝ｅ湪杩愯鐨勬椂鍊?杩涘叆姝や腑鏂簲璇ョ珛鍗冲叧闂緭鍑?        motor_stop();
        motor.restart_delay = BLDC_START_DELAY;
    }
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      瀹氭椂鍣?涓柇鍑芥暟
//  @param      void            
//  @return     void          
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void TM3_Isr(void) interrupt 19
{
	TIM3_CLEAR_FLAG;        // 娓呴櫎涓柇鏍囧織
	T4T3M &= ~0x08;         // 鍋滄瀹氭椂鍣

    // ========== 棰勫畾浣嶉樁娈?(Stage 1) ==========
    if(1 == motor.run_flag)
    {
        // 棰勫畾浣嶆椂闂村凡鍒帮紝杩涘叆寮€鐜姞閫熼樁娈

        motor.run_flag = 2;  // 杩涘叆寮€鐜姞閫熼樁娈

        motor.start_phase = 2;
        motor.open_loop_step = 0;
        motor.duty_ramp_timer = 0;

        // 璁剧疆鐩爣鍗犵┖姣?鏍规嵁鐢垫睜鐢靛帇鍜岄厤缃绠

        motor.target_duty = ((float)BLDC_START_VOLTAGE * BLDC_MAX_DUTY / ((float)battery_voltage/1000));
        if(motor.target_duty > BLDC_MAX_DUTY) motor.target_duty = BLDC_MAX_DUTY;

        // 璁＄畻鍒濆寤舵椂
        motor.motor_start_delay = calculate_acceleration_curve(0, START_OPEN_LOOP_STEP_NUM);

        // 寮€濮嬬涓€娆℃崲鐩

        motor.degauss_flag = 0;
        motor_next_step();
        motor_commutation();
        tim3_reconfig((uint16)motor.motor_start_delay);
        return;
    }

    // ========== 寮€鐜姞閫熼樁娈?(Stage 2) ==========
    if(2 == motor.run_flag)
    {
        motor.open_loop_step++;

        // 妫€鏌ユ槸鍚﹀畬鎴愬紑鐜姞閫

        if(motor.open_loop_step >= START_OPEN_LOOP_STEP_NUM)
        {
            // 寮€鐜鏁扮敤瀹岋紝妫€鏌ユ槸鍚﹀簲璇ヨ繘鍏ヨ繃娓￠樁娈

            if(motor.zc_detect_count >= START_SWITCH_ZC_NUM)
            {
                // 妫€娴嬪埌瓒冲杩囬浂淇″彿锛岃繘鍏ヨ繃娓￠樁娈

                motor.run_flag = 3;
                motor.start_phase = 3;
                motor.motor_start_wait = 0;
            }
            else
            {
                // 鏈娴嬪埌瓒冲杩囬浂淇″彿锛岀户缁繍琛屾垨鍒ゅ畾澶辫触
                motor.motor_start_wait++;
                if(motor.motor_start_wait > BLDC_START_WAIT_TIME)
                {
                    // 鍚姩澶辫触
                    motor_stop();
                    motor.restart_delay = BLDC_START_DELAY;
                    return;
                }
            }
        }

        // 缁х画寮€鐜崲鐩

        motor.degauss_flag = 0;
        motor_next_step();
        motor_commutation();

        // 鏇存柊寤舵椂(浣跨敤鍔犻€熸洸绾

        motor.motor_start_delay = calculate_acceleration_curve(motor.open_loop_step, START_OPEN_LOOP_STEP_NUM);
        tim3_reconfig((uint16)motor.motor_start_delay);

        // 鍗犵┖姣旀枩鍧″鍔?        duty_ramp_up();

        return;
    }
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      瀹氭椂鍣?涓柇鍑芥暟
//  @param      void            
//  @return     void          
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void TM0_Isr(void) interrupt 1
{
    // 鍏抽棴瀹氭椂鍣?    TR0 = 0;

    if(2 == motor.degauss_flag)
    {
        // 瀹屾垚娑堢
        motor.degauss_flag = 0;
    }
}
//-------------------------------------------------------------------------------------------------------------------
//  @brief      PWMA涓柇鍑芥暟
//  @param      void            
//  @return     void          
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void pwma_isr(void) interrupt 26            // PWM涓柇鍑芥暟, 鏍规嵁PWM鍒濆鍖栬缃紝涓柇浼氬湪PWM楂樼數骞崇殑涓棿瑙﹀彂
{
    static uint8 last_comparator_result = 0;
    static uint8 comparator_result = 0;

    uint16 temp;
    uint16 max, min, average;

    if(PWMA_SR1 & 0x10)
    {
        PWMA_SR1 = 0;       // 娓呴櫎鏍囧織浣?        last_comparator_result = comparator_result;
        comparator_result = comparator_result_get();

        if(last_comparator_result != comparator_result)
        {
            if(0 == motor.degauss_flag)
            {
                // 鑾峰彇鎹㈢浉鏃堕棿
                temp = tim4_reconfig();

                // 鍘绘帀鏈€鏃╃殑鏁版嵁
                motor.commutation_time_sum -= motor.commutation_time[motor.step];
                
                // 淇濆瓨鎹㈢浉鏃堕棿
                motor.commutation_time[motor.step] = temp;
                
                // 鍙犲姞鏂扮殑鎹㈢浉鏃堕棿锛屾眰6娆℃崲鐩告€绘椂闀

                motor.commutation_time_sum += temp;
                
                // 鏈€杩?娆℃崲鐩告渶澶у€间笌鏈€灏忓€?                motor_max_min_pwm(motor.commutation_time, 6, &max, &min);
                
                // 6娆℃崲鐩哥殑骞冲潎鍊

                average = motor.commutation_time_sum / 6;
                
                // ========== 寮€鐜姞閫熼樁娈?(Stage 2): 妫€娴嬭繃闆朵絾涓嶅垏鎹紝鍙粺璁?==========
                if(2 == motor.run_flag)
                {
                    uint8 zc_quality_ok = 0;
                    uint16 err_percent = 0;
                    uint16 divisor = 0;

                    motor.commutation_num++;
                    motor.zc_detect_count++;  // 缁熻杩囬浂淇″彿娆℃暟

                    // 妫€鏌ヨ繃闆朵俊鍙疯川閲�
                    divisor = (max + min) >> 1;
                    if((max > 0) && (min > 0) && (divisor > 0))
                    {
                        err_percent = ((max - min) * 100) / divisor;
                        if(err_percent < START_SWITCH_ZC_MAX_ERR)
                        {
                            zc_quality_ok = 1;
                        }
                    }
                    else
                    {
                        err_percent = 0;  // 闄ゅ櫒鏁板紓甯革紝璁句负0琛ㄧず寮傚父
                    }

                    // 妫€鏌ユ槸鍚︽弧瓒冲垏鎹㈡潯浠讹紙浣嗕笉鍦ㄨ繖閲屽垏鎹紝鐢卞畾鏃跺櫒3缁熶竴澶勭悊锛?                    // zc_quality_ok鍜宔rr_percent鐢ㄤ簬鍒ゆ柇锛屼絾鍒囨崲閫昏緫鍦═M3_Isr涓?                    (void)zc_quality_ok;  // 鏄惧紡鏍囪宸蹭娇鐢紝娑堥櫎璀﹀憡
                    (void)err_percent;    // 鏄惧紡鏍囪宸蹭娇鐢紝娑堥櫎璀﹀憡

                    // 鍫佃浆妫€娴

                    if(((max >> 1) > average) || ((min << 1) < average) || (15000 < max) || (100 > min))
                    {
                        motor.commutation_failed_num++;
                    }
                    else if(motor.commutation_failed_num)
                    {
                        motor.commutation_failed_num--;
                    }

                    if(BLDC_COMMUTATION_FAILED_MAX < motor.commutation_failed_num)
                    {
                        motor_stop();
                        motor.restart_delay = BLDC_START_DELAY;
                    }
                    else
                    {
                        motor.degauss_flag = 2;
                        motor_next_step();
                        motor_commutation();
                        timer0_reconfig(65535 - average/4);
                    }
                }
                // ========== 杩囨浮闃舵 (Stage 3): 鍑嗗鍒囨崲鍒版瘮杈冨櫒涓柇 ==========
                else if(3 == motor.run_flag)
                {
                    motor.commutation_num++;

                    // 鍦ㄨ繃娓￠樁娈电疮璁¤冻澶熸崲鐩告鏁板悗锛屾寮忓垏鎹㈠埌闂幆
                    if(motor.motor_start_wait < START_OPEN_LOOP_STABLE_NUM)
                    {
                        motor.motor_start_wait++;

                        // 鍫佃浆妫€娴

                        if(((max >> 1) > average) || ((min << 1) < average) || (15000 < max) || (100 > min))
                        {
                            motor.commutation_failed_num++;
                        }
                        else if(motor.commutation_failed_num)
                        {
                            motor.commutation_failed_num--;
                        }

                        if(BLDC_COMMUTATION_FAILED_MAX < motor.commutation_failed_num)
                        {
                            motor_stop();
                            motor.restart_delay = BLDC_START_DELAY;
                        }
                        else
                        {
                            motor.degauss_flag = 2;
                            motor_next_step();
                            motor_commutation();
                            timer0_reconfig(65535 - average/4);
                        }
                    }
                    else
                    {
                        // 杩囨浮瀹屾垚锛屽垏鎹㈠埌闂幆杩愯
                        pwm_isr_close();
                        motor.run_flag = 4;  // 闂幆杩愯闃舵
                        motor.start_phase = 4;
                        CMPCR1 &= ~0x40;  // 娓呴櫎姣旇緝鍣ㄦ爣蹇椾綅
                    }
                }
                // ========== 棰勫畾浣嶉樁娈?(Stage 1): 涓嶆娴嬭繃闆?==========
                // run_flag=1鏃朵笉澶勭悊杩囬浂淇″彿锛岀瓑寰呭畾鏃跺櫒3杩涘叆寮€鐜樁娈?
            }
        }
    }

}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      姣旇緝鍣ㄤ腑鏂嚱鏁?
//  @param      void
//  @return     void
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void comparator_isr(void) interrupt 21		// 姣旇緝鍣ㄤ腑鏂嚱鏁? 妫€娴嬪埌鍙嶇數鍔ㄥ娍杩?浜嬩欢
{
    uint16 temp;
    uint16 max, min, average;
    
    CMPCR1 &= ~0x40;	// 闇€杞欢娓呴櫎涓柇鏍囧織浣

    // 闂幆杩愯闃舵 (Stage 4): 浣跨敤姣旇緝鍣ㄤ腑鏂繘琛屾崲鐩告帶鍒

    if(0 == motor.degauss_flag && 4 == motor.run_flag)
    {
        // 鑾峰彇鎹㈢浉鏃堕棿
        temp = tim4_reconfig();

        // 鍘绘帀鏈€鏃╃殑鏁版嵁
        motor.commutation_time_sum -= motor.commutation_time[motor.step];
        
        // 淇濆瓨鎹㈢浉鏃堕棿
        motor.commutation_time[motor.step] = temp;
        
        // 鍙犲姞鏂扮殑鎹㈢浉鏃堕棿锛屾眰6娆℃崲鐩告€绘椂闀

        motor.commutation_time_sum += temp;
        
        // 鏈€杩?娆℃崲鐩告渶澶у€间笌鏈€灏忓€

        motor_max_min_comparator(motor.commutation_time, 6, &max, &min);
        
        // 6娆℃崲鐩哥殑骞冲潎鍊

        average = motor.commutation_time_sum / 6;

        motor.commutation_num++;
        // 濡傛灉骞冲潎鍊煎皬浜庢渶澶у€肩殑涓€鍗?鎴栬€?骞冲潎鍊煎ぇ浜庢渶灏忓€肩殑涓ゅ€?鍒欒涓哄牭杞

        if(((max >> 1) > average) || ((min << 1) < average) || (15000 < max) || (100 > min))
        {
            // 妫€娴嬪埌鍫佃浆浜嬩欢
            motor.commutation_failed_num++;
        }
        else if(motor.commutation_failed_num)
        {
            // 娌℃湁妫€娴嬪埌鍫佃浆浜嬩欢 璁板綍鍊煎噺1
            motor.commutation_failed_num--;
        }
        
        if(BLDC_COMMUTATION_FAILED_MAX < motor.commutation_failed_num)
        {
            // 鍫佃浆娆℃暟杈惧埌涓€瀹氭鏁?            motor_stop();
            motor.restart_delay = BLDC_START_DELAY;
        }                                                                                                                                                                                                                                                       
        else
        {
            if(0 == (motor.commutation_num%6))
            {
                if(motor.duty > motor.duty_register)
                {
                    motor.duty_register += 1;
                    if(BLDC_MAX_DUTY < motor.duty_register)
                    {
                        motor.duty_register = BLDC_MAX_DUTY;
                    }
                }
                else
                {
                    motor.duty_register = motor.duty;
                }
                pwm_center_duty_update(motor.duty_register);
            }
            
            // 淇敼PWM鍗犵┖姣

            motor.degauss_flag = 2;
            motor_next_step();
            motor_commutation();
            // 璁剧疆涓€瀹氱殑寤舵椂锛屽欢鏃跺埌涔嬪悗鎵嶅垽鏂繃闆朵俊鍙?            timer0_reconfig(65535 - average/4);
        }
    }
}

#define MUSIC_NUM   15
uint16  frequency_spectrum[6] = {0, 523, 587, 659, 698, 783};
uint8   music_frequency[MUSIC_NUM] = {3,3,4,5,5,4,3,2,1,1,2,3,3,2,2};
uint16  music_wait_time = 250;
//-------------------------------------------------------------------------------------------------------------------
//  @brief      鐢垫満涓婄數楦ｅ彨
//  @param      volume          楦ｅ彨闊抽噺澶у皬            
//  @return     void          
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void motor_power_on_beep(uint16 volume)
{
    int8 i;
    uint16 beep_duty;

    beep_duty = volume;
    
    // 淇濇姢闄愬埗锛岄伩鍏嶈缃繃澶х儳姣佺數鏈

    if(100 < beep_duty)
    {
        beep_duty = 100;
    }

    PWM_A_H_PIN = 0;
	PWM_A_L_PIN = 0;
	PWM_B_H_PIN = 0;
	PWM_B_L_PIN = 0;
	PWM_C_H_PIN = 0;
	PWM_C_L_PIN = 0;
    
    PWM_B_L_PIN = 1;
    for(i = 0; i < MUSIC_NUM; i++)
    {
        pwm_init(PWMA_CH1P_P20, frequency_spectrum[music_frequency[i]], beep_duty);
        //pwm_init(PWMA_CH3P_P24, frequency_spectrum[music_frequency[i]], beep_duty);
        delay_ms(music_wait_time);
    }
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      鐢垫満鍋滄
//  @param      void                        
//  @return     void          
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void motor_stop(void)
{
    motor.run_flag = 0;
    pwm_center_duty_update(0);
    pwm_brake();
    comparator_close_isr();
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      鐢垫満鍚姩
//  @param      void                        
//  @return     void          
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void motor_start(void)
{
    uint16 i;
    uint16 base_duty;

    // ========== 绗竴闃舵锛氬垵濮嬪寲涓庡埞杞?==========
    motor.run_flag = 1;       // 杩涘叆棰勫畾浣嶉樁娈

    motor.start_phase = 1;    // 鏍囪涓洪瀹氫綅闃舵

    // 杞姩涔嬪墠鍏堝埞杞︼紝纭繚鐢垫満闈欐
    pwm_brake();
    delay_ms(100);

    // 娓呴櫎鎵€鏈夌姸鎬佸彉閲?    motor.motor_start_wait = 0;
    motor.commutation_num = 0;
    motor.open_loop_step = 0;
    motor.zc_detect_count = 0;
    motor.duty_ramp_timer = 0;
    for(i=0; i<6; i++)
    {
        motor.commutation_time[i] = 0;
    }
    motor.commutation_time_sum = 0;
    motor.commutation_failed_num = 0;
    motor.degauss_flag = 0;

    // 鍏抽棴杈撳嚭骞舵墦寮€PWM涓柇
    pwm_close_output();
    pwm_isr_open();

    // ========== 绗簩闃舵锛氳浆瀛愰瀹氫綅 (Alignment) ==========
    motor.step = 0;

    // 璁＄畻棰勫畾浣嶅崰绌烘瘮 (浣跨敤閰嶇疆鐨勬瘮渚嬬郴鏁

    base_duty = ((float)BLDC_START_VOLTAGE * BLDC_MAX_DUTY / ((float)battery_voltage/1000));
    motor.align_duty = (uint16)(base_duty * START_ALIGN_DUTY_RATIO);
    if(motor.align_duty < 10) motor.align_duty = 10;  // 鏈€灏忓崰绌烘瘮淇濇姢

    motor.duty_register = motor.align_duty;
    motor.target_duty = base_duty;  // 淇濆瓨鐩爣鍗犵┖姣旂敤浜庡悗缁枩鍧″鍔?    pwm_center_duty_update(motor.duty_register);

    // 鎵ц棰勫畾浣嶆崲鐩?    motor_commutation();

    // 閰嶇疆瀹氭椂鍣紝鍦ㄩ瀹氫綅鏃堕棿鍚庤繘鍏ュ紑鐜姞閫?    tim3_reconfig(START_ALIGN_TIME_MS * 1000);  // 杞崲涓簎s
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      鐢垫満鍒濆鍖?//  @param      void                        
//  @return     void          
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void motor_init(void)
{
    // 鍙橀噺娓呴浂
    motor.duty = 0;
    motor.duty_register = 0;
    motor.run_flag = 0;
    motor.start_phase = 0;
    motor.degauss_flag = 0;
    motor.motor_start_delay = 0;
    motor.motor_start_wait = 0;
    motor.restart_delay = 0;
    motor.commutation_time_sum = 0;
    motor.commutation_num = 0;
    motor.align_duty = 0;
    motor.target_duty = 0;
    motor.zc_detect_count = 0;
    motor.open_loop_step = 0;
    motor.duty_ramp_timer = 0;

    // T0 T1 12T妯″紡
    AUXR &= ~(0X03 << 6);
    
    // T3 T4 12T妯″紡
    T4T3M &= ~0x22; 
    
    // 浣胯兘瀹氭椂鍣?銆?涓柇
    IE2 |= 0x60;
    
    // 浣胯兘瀹氭椂鍣?涓柇
    ET0 = 1;
    
    IP = 0;
    IPH = 0;
    IP2 = 0;
    IP2H = 0;
    
    // 璁剧疆姣旇緝鍣ㄤ笌PWM鐨勪腑鏂负鏈€楂樹紭鍏堢骇
    IP |= 1<<7;
    IPH |= 1<<7;
    
    IP2 |= 1<<5;
    IP2H |= 1<<5;
    
#if (1 == BLDC_BEEP_ENABLE)
    // 鐢垫満楦ｅ彨琛ㄧず鍒濆鍖栧畬鎴?    motor_power_on_beep(BLDC_BEEP_VOLUME);  
#endif
}
