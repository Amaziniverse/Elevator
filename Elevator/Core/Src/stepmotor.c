#include "main.h"
#include "button.h"
#include "i2c_lcd.h"

void set_rpm(int rpm);
void stepmotor_main_test();
void stepmotor_drive(int step);

extern void delay_us(unsigned long us);
extern int get_button(GPIO_TypeDef *GPIO, uint16_t GPIO_PIN, uint8_t button_number);
extern void i2c_lcd_init(void);
extern void lcd_string(uint8_t *str);
extern void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
extern void move_cursor(uint8_t row, uint8_t column);

int current_floor = 0;

// RPM : 분당회전수
// 1분: 60sec: 1,000,000us(1초) x 60 = 60,000,000us
// 1,000,000us (1초)
// --> 1초(1000ms) ==>(1000us) x 1000ms ==> 1,000,000us
// 4096 스텝 : 1바퀴 (4096스텝이동)
// --> 1바퀴 도는데 4096스텝필요
// 4096 / 8(0.7) ==> 512 sequence : 360도
// 1 sequence(8step) : 0.70312도
// 0.70312도 x 512 = 360도

// -------- set_rpm(13) 으로 지정시의 동작상황 ---------
// 60,000,000us(1분) / 4096 / rpm
// 1126us(1스텝 idle 타입) x 4096 = 4,612,096us
//                        = 4612ms
//                        = 4.6초
// 60 / 4.6 (1회전 소요시간 초) ==> 13회전
// 시계방향으로 1회전 <---> 반시계방향으로 1회전
void set_rpm(int rpm)
{
   delay_us(60000000/4096/rpm);
   // 최대 speed 기준 (13) : delay_us(1126)
}

void stepmotor_main_test()
{
   static int moving_status = 0;
   static int direction_status = 1;
   static int button0_count = 0;
   static int button1_count = 0;
   static int button2_count = 0;
   char lcd_buffer[40];

   // 움직임 방향에 따른 속도 제어
   if (moving_status) {
      stepmotor_drive(direction_status);
      if (direction_status==1)	// 위
         set_rpm(13);
      if (direction_status==2)	 // 아래
         set_rpm(13);
      else if (direction_status==0)  // 정지
    	 set_rpm(0);
   }

   else	{
	   set_rpm(0);
   }

#if 1
   // 버튼 처리에 의한 스텝모터 구동 프로그램 작성
   // button0 한 번 누르면 위로, 또 한 번 누르면 아래로
   // button1 짝수층 운행
   // button2 홀수층 운행
   // button3 급정지 버튼
   if (get_button(BUTTON0_GPIO_Port, BUTTON0_Pin, 0) == BUTTON_PRESS) // BUTTON0
   {
      button0_count ++;
      button0_count %= 2;

      if(button0_count) {
    	 moving_status = 1;
    	 move_cursor(1, 0);
    	 sprintf(lcd_buffer, "      down      ");
    	 lcd_string(lcd_buffer);
         direction_status = 2;
      }
      else {
    	 moving_status = 1;
    	 move_cursor(1, 0);
         sprintf(lcd_buffer, "       up      ");
    	 lcd_string(lcd_buffer);
         direction_status = 1;
      }
   }

   if (get_button(BUTTON0_GPIO_Port, BUTTON1_Pin, 1) == BUTTON_PRESS) // BUTTON1
      {
         button1_count ++;
         button1_count %= 2;

         if(button1_count) {
        	 moving_status = 1;
        	 move_cursor(1, 0);
        	 sprintf(lcd_buffer, "    1st floor  ");
        	 lcd_string(lcd_buffer);

        	 if (current_floor==1) {
        		direction_status = 0;
        	 }
        	 if (current_floor==2) {
        		direction_status = 2;
        	 }
        	 if (current_floor==3) {
        		direction_status = 2;
        	 }
        	 else if (current_floor==4) {
        		direction_status = 2;
        	 }
         }

         else {
        	moving_status = 1;
        	move_cursor(1, 0);
        	sprintf(lcd_buffer,"    3rd floor  ");
        	lcd_string(lcd_buffer);

        	if (current_floor==1) {
				direction_status = 1;
			}
			if (current_floor==2) {
				direction_status = 1;
			}
			if (current_floor==3) {
				direction_status = 0;
			}
			else if (current_floor==4) {
				direction_status = 2;
			}
         }
      }

   if (get_button(BUTTON0_GPIO_Port, BUTTON2_Pin, 2) == BUTTON_PRESS) // BUTTON2
      {
		 button2_count ++;
		 button2_count %= 2;

		 if(button2_count) {
			 moving_status = 1;
			 move_cursor(1, 0);
			 sprintf(lcd_buffer, "    2nd floor  ");
			 lcd_string(lcd_buffer);

			 if (current_floor==1) {
				direction_status = 1;
			 }
			 if (current_floor==2) {
				 direction_status = 0;
			 }
			 if (current_floor==3) {
				direction_status = 2;
			 }
			 else if (current_floor==4) {
				direction_status = 2;
			 }
		 }

		else {
			moving_status = 1;
			move_cursor(1, 0);
			sprintf(lcd_buffer,"    4th floor  ");
			lcd_string(lcd_buffer);

			if (current_floor==1) {
				direction_status = 1;
			}
			if (current_floor==2) {
				direction_status = 1;
			}
			if (current_floor==3) {
				direction_status = 1;
			}
			else if (current_floor==4) {
				direction_status = 0;
			}
		  }
       }

   if (get_button(BUTTON0_GPIO_Port, BUTTON3_Pin, 3) == BUTTON_PRESS) // BUTTON3 = stop
   {
	  i2c_lcd_init();
      moving_status = 0;
   }


#else

   for(int i=0; i<512; i++) // 시계방향 1회전
   {
      for(int j=0; j<8; j++)
      {
         stepmotor_drive(j);
         set_rpm(13);
      }
   }
   for(int i=0; i<512; i++) // 반시계방향 1회전
   {
      for(int j=7; j>=0; j--)
      {
         stepmotor_drive(j);
         set_rpm(13); // rpm값만큼 wait
      }
   }
#endif
}

#if 1
//
// direction : 1 --> 시계방향
//           : 2 --> 반시계
//            0 : idle
void stepmotor_drive(int direction)
{
   static int step=0; // static을 쓰면 전역변수 처럼 동작

   switch(step){
   case 0:
      HAL_GPIO_WritePin(GPIOD, IN1_Pin, 1);
      HAL_GPIO_WritePin(GPIOD, IN2_Pin, 0);
      HAL_GPIO_WritePin(GPIOD, IN3_Pin, 0);
      HAL_GPIO_WritePin(GPIOD, IN4_Pin, 0);
      break;
   case 1:
      HAL_GPIO_WritePin(GPIOD, IN1_Pin, 1);
      HAL_GPIO_WritePin(GPIOD, IN2_Pin, 1);
      HAL_GPIO_WritePin(GPIOD, IN3_Pin, 0);
      HAL_GPIO_WritePin(GPIOD, IN4_Pin, 0);
      break;
   case 2:
      HAL_GPIO_WritePin(GPIOD, IN1_Pin, 0);
      HAL_GPIO_WritePin(GPIOD, IN2_Pin, 1);
      HAL_GPIO_WritePin(GPIOD, IN3_Pin, 0);
      HAL_GPIO_WritePin(GPIOD, IN4_Pin, 0);
      break;
   case 3:
      HAL_GPIO_WritePin(GPIOD, IN1_Pin, 0);
      HAL_GPIO_WritePin(GPIOD, IN2_Pin, 1);
      HAL_GPIO_WritePin(GPIOD, IN3_Pin, 1);
      HAL_GPIO_WritePin(GPIOD, IN4_Pin, 0);
      break;
   case 4:
      HAL_GPIO_WritePin(GPIOD, IN1_Pin, 0);
      HAL_GPIO_WritePin(GPIOD, IN2_Pin, 0);
      HAL_GPIO_WritePin(GPIOD, IN3_Pin, 1);
      HAL_GPIO_WritePin(GPIOD, IN4_Pin, 0);
      break;
   case 5:
      HAL_GPIO_WritePin(GPIOD, IN1_Pin, 0);
      HAL_GPIO_WritePin(GPIOD, IN2_Pin, 0);
      HAL_GPIO_WritePin(GPIOD, IN3_Pin, 1);
      HAL_GPIO_WritePin(GPIOD, IN4_Pin, 1);
      break;
   case 6:
      HAL_GPIO_WritePin(GPIOD, IN1_Pin, 0);
      HAL_GPIO_WritePin(GPIOD, IN2_Pin, 0);
      HAL_GPIO_WritePin(GPIOD, IN3_Pin, 0);
      HAL_GPIO_WritePin(GPIOD, IN4_Pin, 1);
      break;
   case 7:
      HAL_GPIO_WritePin(GPIOD, IN1_Pin, 1);
      HAL_GPIO_WritePin(GPIOD, IN2_Pin, 0);
      HAL_GPIO_WritePin(GPIOD, IN3_Pin, 0);
      HAL_GPIO_WritePin(GPIOD, IN4_Pin, 1);
      break;
   }
   if (direction == 1) // 정회전
   {
      step++; // for (step=0; step<8; step++)
      step %= 8; // 다음 진행할 step 준비
      // if (step >= 8) step = 0;
   }
   else if (direction == 2) // 역회전
   {
      step--;
      if(step < 0) step = 7;
   }
}
#else
void stepmotor_drive(int step)
{
   switch(step){
   case 0:
      HAL_GPIO_WritePin(GPIOD, IN1_Pin, 1);
      HAL_GPIO_WritePin(GPIOD, IN2_Pin, 0);
      HAL_GPIO_WritePin(GPIOD, IN3_Pin, 0);
      HAL_GPIO_WritePin(GPIOD, IN4_Pin, 0);
      break;
   case 1:
      HAL_GPIO_WritePin(GPIOD, IN1_Pin, 1);
      HAL_GPIO_WritePin(GPIOD, IN2_Pin, 1);
      HAL_GPIO_WritePin(GPIOD, IN3_Pin, 0);
      HAL_GPIO_WritePin(GPIOD, IN4_Pin, 0);
      break;
   case 2:
      HAL_GPIO_WritePin(GPIOD, IN1_Pin, 0);
      HAL_GPIO_WritePin(GPIOD, IN2_Pin, 1);
      HAL_GPIO_WritePin(GPIOD, IN3_Pin, 0);
      HAL_GPIO_WritePin(GPIOD, IN4_Pin, 0);
      break;
   case 3:
      HAL_GPIO_WritePin(GPIOD, IN1_Pin, 0);
      HAL_GPIO_WritePin(GPIOD, IN2_Pin, 1);
      HAL_GPIO_WritePin(GPIOD, IN3_Pin, 1);
      HAL_GPIO_WritePin(GPIOD, IN4_Pin, 0);
      break;
   case 4:
      HAL_GPIO_WritePin(GPIOD, IN1_Pin, 0);
      HAL_GPIO_WritePin(GPIOD, IN2_Pin, 0);
      HAL_GPIO_WritePin(GPIOD, IN3_Pin, 1);
      HAL_GPIO_WritePin(GPIOD, IN4_Pin, 0);
      break;
   case 5:
      HAL_GPIO_WritePin(GPIOD, IN1_Pin, 0);
      HAL_GPIO_WritePin(GPIOD, IN2_Pin, 0);
      HAL_GPIO_WritePin(GPIOD, IN3_Pin, 1);
      HAL_GPIO_WritePin(GPIOD, IN4_Pin, 1);
      break;
   case 6:
      HAL_GPIO_WritePin(GPIOD, IN1_Pin, 0);
      HAL_GPIO_WritePin(GPIOD, IN2_Pin, 0);
      HAL_GPIO_WritePin(GPIOD, IN3_Pin, 0);
      HAL_GPIO_WritePin(GPIOD, IN4_Pin, 1);
      break;
   case 7:
      HAL_GPIO_WritePin(GPIOD, IN1_Pin, 1);
      HAL_GPIO_WritePin(GPIOD, IN2_Pin, 0);
      HAL_GPIO_WritePin(GPIOD, IN3_Pin, 0);
      HAL_GPIO_WritePin(GPIOD, IN4_Pin, 1);
      break;
   }
}
#endif

