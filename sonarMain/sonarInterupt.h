//Handles polling of the Sonar Sensors


#ifndef SONAR_H
#define SONAR_H
//////////////////////////////////////////////////////////////////////////
////					SENSOR CONFIG DEFINITIONS					//////
//////////////////////////////////////////////////////////////////////////

//The Number of Sonar Sensors Used
#define nSonar 8

//Define Sonar Pins
#define SonarTrig (1 << PD4)

//Define the Error Value used to indicate invalid readings
#define SonarError 65355

//Define the Reading the Sonar can accurately make
#define MaxDistance 65355	//Per the Data sheet the sonar can only measure up to 3 m

//////////////////////////////////////////////////////////////////////////

//Include General Libraries for interfacing with Arduino
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

//Initialize volatile long array to store Sonar Readings
volatile unsigned long SonarReading[nSonar] = {0};
	
//Initialize volatile to store new Sonar Reading pre-verification
volatile unsigned long SonarReadingNew = 0;

//Initialize volatile to store start times of the echo pulse
volatile unsigned long SonarReadingStart = 0;

//Initialize register to store prior states of PORT
volatile unsigned short SonarPinsLast = 0;

///////////////////////////////////////////////////////////////////////////////
// Define Sonars Pins

// Sets the I/O Port of each sonar Pin
const char sonarPort[nSonar] = {'D', 'B', 'B', 'B', 'B', 'B', 'C', 'C'};

// Sets the position on the I/O Port of each sonar pin
const short sonarPin[nSonar] = {(1<<7), (1<<0), (1<<1), (1<<2), (1<<3), (1<<4), (1<<PCINT8), (1<<PCINT9)};


///////////////////////////////////////////////////////////////////////////////

// Index of the Current Sonar -> Start at -1 to fire sonar 0 first
int curSonar = -1;

//Interrupt to handle cleanup of sonar reading and initialize the next reading
ISR(TIMER1_COMPA_vect){
	// Move to the next sonar
	int priorSonar = curSonar;
	curSonar = (curSonar+1) % nSonar;

	// Disable Interrupts for Prior Sonar
	if (sonarPort[priorSonar] == 'B'){
		PCMSK0 &= ~sonarPin[priorSonar];
	} else if (sonarPort[priorSonar] == 'C'){
		PCMSK1 &= ~sonarPin[priorSonar];
	} else if (sonarPort[priorSonar] == 'D'){
		PCMSK2 &= ~sonarPin[priorSonar];
	}
		
	// If Sonar Measurement > MaxDistance Or Zero -> Bad Reading
	if((SonarReadingNew >= MaxDistance) || (SonarReadingNew == 0)){
		SonarReadingNew = SonarError;
	}

	//Transfer verified reading to SonarReading
	SonarReading[priorSonar] = SonarReadingNew;

	// Reset SonarReading New
	SonarReadingNew = 0;

	//////////////////////////////////////////////////////////////////////////
	//Trigger the Next Sonar Readings
	//////////////////////////////////////////////////////////////////////////
	//Set Sonar Trigger Pins to Output
	DDRD |= SonarTrig;
	
	//Set Sonar Trigger Pin low to force clean pulse
	PORTD &= ~SonarTrig;
	
	// Wait 2 uS
	_delay_us(2);

	// Set Sonar Trigger Pin High to start Pulse
	PORTD |= SonarTrig;
	
	//Wait 5 uS per Sonar Data sheet
	_delay_us(5);
	
	//Set Sonar Trigger Pin Low to end pulse
	PORTD &= ~SonarTrig;

	//////////////////////////////////////////////////////////////////////////
	
	//Set Current Sonar Echo Pin to Input
	if (sonarPort[curSonar] == 'B'){
		DDRB &= ~sonarPin[curSonar];
	} else if (sonarPort[curSonar] == 'C'){
		DDRC &= ~sonarPin[curSonar];
	} else if (sonarPort[curSonar] == 'D'){
		DDRD &= ~sonarPin[curSonar];
	}
	
	//Enable Pin Change Interrupts for current Sonar
	if (sonarPort[curSonar] == 'B'){
		PCMSK0 |= sonarPin[curSonar];
	} else if (sonarPort[curSonar] == 'C'){
		PCMSK1 |= sonarPin[curSonar];
	} else if (sonarPort[curSonar] == 'D'){
		PCMSK2 |= sonarPin[curSonar];
	}

	//Set Timer 1 to Zero
	TCNT1 = 0;
}

//ISR To Handle Detecting and Recording the Response Pulse from the Sonar Sensors
ISR(PCINT0_vect){
	//Read Port B
	uint8_t PinsCurrent = PINB;
	
	//////////////////////////////////////////////////////////////////////////
	//Detect Rising Edges
	//////////////////////////////////////////////////////////////////////////
	
	//Rising Edge if Pin is Currently High and used to be Low
	uint8_t PinRisingEdge = ~SonarPinsLast & PinsCurrent;
	if(PinRisingEdge & sonarPin[curSonar]){
		//Start of Pulse on Sonar Pin Received -> Record in SonarReadingStart Array
		SonarReadingStart = TCNT1;
	}
	
	//////////////////////////////////////////////////////////////////////////
	//Detect Falling Edges
	//////////////////////////////////////////////////////////////////////////
	
	//Failing Edge if Pin is Currently Low and Used to be High
	uint8_t PinFallingEde = ~PinsCurrent & SonarPinsLast;
	if(PinFallingEde & sonarPin[curSonar]){
		//End of Pulse on Sonar Pin Received -> Compute Elapsed Time
		SonarReadingNew = TCNT1 - SonarReadingStart;
	}
	
	//////////////////////////////////////////////////////////////////////////
	
	//Update SonarPinsLast to Current Pin State
	SonarPinsLast = PinsCurrent;
}
ISR(PCINT1_vect){
	//Read Port C
	uint8_t PinsCurrent = PINC;
	
	//////////////////////////////////////////////////////////////////////////
	//Detect Rising Edges
	//////////////////////////////////////////////////////////////////////////
	
	//Rising Edge if Pin is Currently High and used to be Low
	uint8_t PinRisingEdge = ~SonarPinsLast & PinsCurrent;
	if(PinRisingEdge & sonarPin[curSonar]){
		//Start of Pulse on Sonar Pin Received -> Record in SonarReadingStart Array
		SonarReadingStart = TCNT1;
	}
	
	//////////////////////////////////////////////////////////////////////////
	//Detect Falling Edges
	//////////////////////////////////////////////////////////////////////////
	
	//Failing Edge if Pin is Currently Low and Used to be High
	uint8_t PinFallingEde = ~PinsCurrent & SonarPinsLast;
	if(PinFallingEde & sonarPin[curSonar]){
		//End of Pulse on Sonar Pin Received -> Compute Elapsed Time
		SonarReadingNew = TCNT1 - SonarReadingStart;
	}
	
	//////////////////////////////////////////////////////////////////////////
	
	//Update SonarPinsLast to Current Pin State
	SonarPinsLast = PinsCurrent;
}
ISR(PCINT2_vect){
	//Read Port D
	uint8_t PinsCurrent = PIND;
	
	//////////////////////////////////////////////////////////////////////////
	//Detect Rising Edges
	//////////////////////////////////////////////////////////////////////////
	
	//Rising Edge if Pin is Currently High and used to be Low
	uint8_t PinRisingEdge = ~SonarPinsLast & PinsCurrent;
	if(PinRisingEdge & sonarPin[curSonar]){
		//Start of Pulse on Sonar Pin Received -> Record in SonarReadingStart Array
		SonarReadingStart = TCNT1;
	}
	
	//////////////////////////////////////////////////////////////////////////
	//Detect Falling Edges
	//////////////////////////////////////////////////////////////////////////
	
	//Failing Edge if Pin is Currently Low and Used to be High
	uint8_t PinFallingEde = ~PinsCurrent & SonarPinsLast;
	if(PinFallingEde & sonarPin[curSonar]){
		//End of Pulse on Sonar Pin Received -> Compute Elapsed Time
		SonarReadingNew = TCNT1 - SonarReadingStart;
	}
	
	//////////////////////////////////////////////////////////////////////////
	
	//Update SonarPinsLast to Current Pin State
	SonarPinsLast = PinsCurrent;
}
#endif /* INCFILE1_H_ */