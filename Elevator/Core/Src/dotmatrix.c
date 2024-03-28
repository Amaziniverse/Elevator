#include "main.h"
#include "cmsis_os.h"

extern void lcd_string(uint8_t *str);
extern void move_cursor(uint8_t row, uint8_t column);
extern myMutex01Handle;

int dotmatrix_main_test(void);
void init_dotmatrix(void);

GPIO_TypeDef *col_port[] =
{
		COL1_GPIO_Port, COL2_GPIO_Port, COL3_GPIO_Port, COL4_GPIO_Port,
		COL5_GPIO_Port, COL6_GPIO_Port, COL7_GPIO_Port, COL8_GPIO_Port

};
GPIO_TypeDef *row_port[] =
{
		ROW1_GPIO_Port, ROW2_GPIO_Port, ROW3_GPIO_Port, ROW4_GPIO_Port,
		ROW5_GPIO_Port, ROW6_GPIO_Port, ROW7_GPIO_Port, ROW8_GPIO_Port

};

uint16_t row_pin[] =
{
		ROW1_Pin, ROW2_Pin, ROW3_Pin, ROW4_Pin,
		ROW5_Pin, ROW6_Pin, ROW7_Pin, ROW8_Pin
};

uint16_t col_pin[] =
{
		COL1_Pin, COL2_Pin, COL3_Pin, COL4_Pin,
		COL5_Pin, COL6_Pin, COL7_Pin, COL8_Pin
};



unsigned char all_on[] = { // all on 문자 정의
#if 0

	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111
#else
	0b00000000,
	0b11100111,
	0b10111101,
	0b10000001,
	0b01000010,
	0b01100010,
	0b00110100,
	0b00011000
#endif
};

const uint8_t blank[8] = {
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000
};

uint8_t number_data[20][10] =
{
	{
		0b01000000,	//1
		0b11000000,
		0b01000000,
		0b01000000,
		0b01000000,
		0b01000000,
		0b11100000,
	    6   // 점 0b00000110
	},
	{
		0b01110000,	//2
		0b10001000,
		0b00001000,
		0b00010000,
		0b00100000,
		0b01000000,
		0b11111000,
	    6
	},
	{
		0b11111000,	//3
	    0b00010000,
		0b00100000,
		0b00010000,
		0b00001000,
		0b10001000,
		0b01110000,
	    6
	},
	{
		0b00010000,	//4
		0b00110000,
		0b01010000,
		0b10010000,
		0b11111000,
		0b00010000,
		0b00010000,
	    6
	},
	{
		0b00000000,    // hart
		0b01100110,
		0b11111111,
		0b11111111,
		0b11111111,
		0b01111110,
		0b00111100,
		0b00011000
	},
	{
		0b00000000,    // E
	    0b00111100,
		0b00100000,
		0b00111100,
		0b00100000,
		0b00100000,
		0b00111100,
		0b00000000
	},
	{
		0b00000000,
		0b00110000,
		0b00110000,
		0b00110000,
		0b00110000,
		0b00110000,
		0b00111110,
		0b00000000
	},
	{
		0b00000000,    // E
		0b00111100,
		0b00100000,
		0b00111100,
		0b00100000,
		0b00100000,
		0b00111100,
		0b00000000
	},
	{
		0b00000000,
		0b01000010,
		0b01000010,
		0b00100100,
		0b00100100,
		0b00011000,
		0b00011000,
		0b00000000
	},
	{
		0b00000000,
		0b00011000,
		0b00011000,
		0b00100100,
		0b00111100,
		0b01000010,
		0b01000010,
		0b00000000
	},
	{
		0b00000000,
		0b01111110,
		0b00011000,
		0b00011000,
		0b00011000,
		0b00011000,
		0b00011000,
		0b00000000
	},
	{
		0b00000000,
		0b00011000,
		0b00100100,
		0b00100100,
		0b00100100,
		0b00100100,
		0b00011000,
		0b00000000
	},
	{
		0b00000000,
		0b00111100,
		0b00100100,
		0b00111100,
		0b00101000,
		0b00100100,
		0b00100010,
		0b00000000
	}
};

unsigned char display_data[8];    // 최종 8*8 출력할 데이터
unsigned char scroll_buffer[50][8] = {0};    // 스크롤할 데이터가 들어있는 버퍼
int number_of_character = 15;    // 출력할 문자 갯수

// 초기화 작업
// 1. display_data 애 number_data[0]에 있는 내용 복사
// 2. number_data를 scroll_buffer에 복사
// 3. dotmatrix의 led를 off
void init_dotmatrix(void)
{
	for (int i=0; i<8; i++)
	{
		display_data[i] = number_data[i];
	}
	for (int i=1; i<number_of_character+1; i++)
	{
		for (int j=0; j<8; j++)    // scroll_buffer[0] = blank
		{
			scroll_buffer[i][j] = number_data[i-1][j];
		}
	}
	for (int i=0; i<8; i++)
	{
		HAL_GPIO_WritePin(col_port[i], col_pin[i], 1);    // 캐소우드 방식은 반대로
	}
}
void write_column_data(int col)
{
	for (int i = 0; i < 8; i++)
	{
#if 1
// -----common 애노우드 방식
		if(i == col)
			HAL_GPIO_WritePin(col_port[i], col_pin[i], 0); // on
		else HAL_GPIO_WritePin(col_port[i], col_pin[i], 1); // off
#else
// -----common 캐소우드 방식
		if(i == col)
					HAL_GPIO_WritePin(col_port[i], col_pin[i], 1); // off
		else HAL_GPIO_WritePin(col_port[i], col_pin[i], 0); // on
#endif
	}
}

// 0b00111100를 찍는다고 칩시다.
void write_row_data(unsigned char data)
{
	unsigned char d; // 임시 변수

	d = data;

	for(int i = 0; i < 8; i++)
	{
		if ( d & (1 << i)) // 00000001인 경우 -> 한 칸씩 옆으로 이동하고 &연산
			HAL_GPIO_WritePin(row_port[i], row_pin[i], 1);  // 10으로 2면 여기로
		else
			HAL_GPIO_WritePin(row_port[i], row_pin[i], 0); // off
	}
}

#if 1
// scroll 문자 출력 프로그램
int dotmatrix_main_test(void)
{
	static int count=0;    // colum count
	static int index=0;    // scrol_buffer의 2차원 index값
	static uint32_t past_time=0;

	char lcd_buff[40];

//	init_dotmatrix();

	uint32_t now = HAL_GetTick();    // 1ms tick이라는 것은 time 값
	// 처음 시작 시 past_time=0;    now: 500 --> past_time=500
	if (now - past_time >= 500)    // 500ms 마다 scroll
	{
		sprintf(lcd_buff, "now: %d", now);
		if (osMutexAcquire(myMutex01Handle, 1000) == osOK) {   // 기존에 lcok이 걸려 있으면 풀릴 때까지 기다린다.
															 // control이 다른 taskfh로 넘어가지 않도록 lock을 건다.
			move_cursor(0, 0);
			lcd_string("====ELEVATOR====");
			osMutexRelease(myMutex01Handle);	// unlock control이 다른 task로 넘어가도록 한다.
		}
		past_time = now;
		for (int i=0; i<8; i++)
		{
			display_data[i] = (scroll_buffer[index][i] >> count) |
					(scroll_buffer[index+1][i] << 8 - count);
		}
		if (++count == 8)    // 8칼람을 다 처리했으면 다음 scroll_buffer로 이동
		{
			count=0;
			index++;   // 다음 scroll_buffer로 이동
			if (index == number_of_character+1) index=0;
			// 11개의 문자를 다 처리했으면 0번 scroll_buffer 처리하기 위해 이동
		}
	}
	for (int i=0; i < 8; i++)
	{
		// 공통 양극 방식
		// column에는 0을 ROW에는 1을 출력해야 해당 LED가 on된다.
		write_column_data(i);
		write_row_data(display_data[i]);
		HAL_Delay(1);
	}
	return 0; // 에러 없이 정상적으로 끝났다.
}
#else
// 고정문자 출력 demo program
int dotmatrix_main_test(void)
{
	while(1)
	{
		for (int i=0; i < 8; i++)
		{
			// 공통 양극 방식
			// column에는 0을 ROW에는 1을 출력해야 해당 LED가 on된다.
			write_column_data(i);
			write_row_data(all_on[i]);

			HAL_Delay(1);
		}
	}
	return 0; // 에러 없이 정상적으로 끝났다.
}
#endif





