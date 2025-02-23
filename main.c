
#ifndef F_CPU
#define F_CPU 16000000UL  // 16MHz 클럭
#endif

#include <avr/io.h>
#include <util/delay.h>

// 핀 정의
#define FAN_PWM_PIN  PE3      // 모터 제어 핀 (OC3A)
#define BUTTON1_PIN  PG2      // 버튼 1: 모터 50% 속도로 켜기
#define BUTTON2_PIN  PG3      // 버튼 2: 모터 끄기
#define BUTTON3_PIN  PG4      // 버튼 3: 모터 속도 조절

// 7-세그먼트 디스플레이 숫자 정의 (공통 애노드 기준)
const uint8_t SEGMENT_NUMS[10] = {
	0b10111111, // 0
	0b10110000, // 1
	0b11111001, // 2
	0b11111001, // 3
	0b11110010, // 4
	0b10010010, // 5
	0b10000010, // 6
	0b11111000, // 7
	0b10000000, // 8
	0b10010000  // 9
};

// 7-세그먼트 디스플레이에 숫자 표시
void display_number(uint8_t num) {
	PORTA = SEGMENT_NUMS[num];
}

// PWM 초기화: Timer3를 Fast PWM 모드, 비반전 모드, 분주비 64로 설정
void pwm_init(void) {
	DDRE |= (1 << FAN_PWM_PIN); // PE3를 출력으로 설정
	TCCR3A = (1 << WGM31) | (1 << COM3A1); // Fast PWM 모드, 비반전 모드
	TCCR3B = (1 << WGM33) | (1 << WGM32) | (1 << CS31) | (1 << CS30); // 분주비 64
	ICR3 = 0xFFFF; // TOP 값 설정 (16비트 최대값)
}

// 모터 속도 설정 (duty_cycle: 0~65535)
void set_motor_speed(uint16_t duty_cycle) {
	OCR3A = duty_cycle;
}

// 버튼 입력 확인 (디바운싱 포함)
uint8_t is_button_pressed(uint8_t pin) {
	if (!(PING & (1 << pin))) {  // 버튼이 눌리면 LOW
		_delay_ms(50); // 디바운싱을 위한 지연
		if (!(PING & (1 << pin))) {
			return 1;
		}
	}
	return 0;
}

// 버튼이 떼어질 때까지 대기
void wait_button_release(uint8_t pin) {
	while (!(PING & (1 << pin))) {
		_delay_ms(20);
	}
}

int main(void) {
	// 포트 설정
	DDRA = 0xFF; // 7-세그먼트 디스플레이 연결된 포트A를 출력으로 설정
	DDRG &= ~((1 << BUTTON1_PIN) | (1 << BUTTON2_PIN) | (1 << BUTTON3_PIN)); // 버튼 핀을 입력으로 설정
	PORTG |= (1 << BUTTON1_PIN) | (1 << BUTTON2_PIN) | (1 << BUTTON3_PIN);   // 내부 풀업 저항 활성화

	pwm_init(); // PWM 초기화
	set_motor_speed(0); // 초기에는 모터 정지
	uint16_t speed = 0; // 모터 속도 (0: 정지)
	uint8_t speed_toggle = 0; // 버튼 3의 누름 상태를 저장

	while (1) {
		if (is_button_pressed(BUTTON1_PIN)) { // 버튼 1 눌림
			display_number(1); // 7-세그먼트에 '1' 표시
			speed = 32768; // 모터 50% 속도
			set_motor_speed(speed);
			wait_button_release(BUTTON1_PIN);
		}

		if (is_button_pressed(BUTTON2_PIN)) { // 버튼 2 눌림
			display_number(0); // 7-세그먼트에 '2' 표시
			speed = 0; // 모터 정지
			set_motor_speed(speed);
			wait_button_release(BUTTON2_PIN);
		}

		if (is_button_pressed(BUTTON3_PIN)) { // 버튼 3 눌림
			speed_toggle = !speed_toggle; // 토글 상태 변경
			if (speed_toggle) {
				display_number(3); // 7-세그먼트에 '3' 표시
				speed = 45845; // 모터 속도 70%
				} else {
				display_number(4); // 7-세그먼트에 '4' 표시
				speed = 55705; // 모터 속도 85%
			}
			set_motor_speed(speed);
			wait_button_release(BUTTON3_PIN);
		}
	}

	return 0;
}
