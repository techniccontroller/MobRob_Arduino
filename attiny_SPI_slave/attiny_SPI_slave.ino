// Includes for ISR and IO operations
#include <avr/interrupt.h>
#include <avr/io.h>

// - Pin definitions for LED & SPI
#define PIN_MOSI 6      // MOSI / PA6
#define PIN_MISO 5      // MISO / PA5
#define PIN_SCK  4      // SCK  / PA4
#define PIN_SS   3      // SSb  / PA3


// Define data direction register for PIN_MISO
#define DDR_MISO_PORT   DDRA
#define DDR_MISO_FIELD  DDA2

// Modules
#define MOD_DEFAULT 20
#define MOD_SERVO 30
#define MOD_VERTICAL 40
#define MOD_GRIPPER 50

// Functions
#define STATUS_CODE 0
#define ACTIVE 5
#define INITIALIZE 10
#define SPEED 20
#define MAX_POS 30
#define MIN_POS 40
#define HOME_POS 50
#define POS_VALUE 60
#define MOVE_2_MIN 70
#define MOVE_2_MAX 80
#define MOVE_2_HOME 90
#define MOVE_REL_MIN 100
#define MOVE_REL_MAX 110
#define ROT_REL_DEG_CLOCK 120
#define ROT_REL_DEG_COUNTER 130

#define REG_MAX 5


// SPI slave

volatile boolean        m_bSlaveSelected = false;
volatile uint16_t       m_nXferCycle = 0;


// Latched command type and reg address from COMMAND cycle
volatile uint8_t    m_nRegMod = 0;
volatile uint8_t    m_nRegFunc = 0;
volatile uint8_t    m_bRegRWb = 0;


// Declare the primary register array
volatile uint8_t m_anRegArray[REG_MAX];



// -----------------------------------------------------------------------
// START OF PROGRAM CODE
// -----------------------------------------------------------------------

// Configure the interfaces, the interrupts and assign
// the default register values
void setup() {

  // Initialize register array
  for (int nRegInd = 0; nRegInd < REG_MAX; nRegInd++) {
    m_anRegArray[nRegInd] = 0x00;
  }
  
  // Setup SPI
  setupConfig();

  // Enable interrupts
  sei();  
}

// Initialize the SPI interface
void setupConfig() {
  
  // Define pins for SPI
  // - Start with the slave unselected
  pinMode(PIN_MISO, INPUT);
  pinMode(PIN_MOSI, INPUT);
  pinMode(PIN_SCK,  INPUT);
  pinMode(PIN_SS,   INPUT);

  // Enable SPI and the SPI Transaction Complete ISR
  SPCR |= (1 << SPE) | (1 << SPIE);
}

// SPI Transaction complete ISR
//
// This ISR is responsible for handling the read and write
// transfer cycles during a SPI transaction.
ISR(SPI_STC_vect)
{

  // Local variables
  uint8_t  nBufDat = 0;      // Storage for incoming SPDR
  uint8_t  nRegCmd = 0;

  // ----------------------------------
  // Capture inputs for previous cycle
  // ----------------------------------

  // Fetch the incoming data byte from the SPI Data Register
  nBufDat = SPDR;
    
  if (m_nXferCycle == 0) {
    // MODULE cycle

    // Latch command
    nRegCmd = nBufDat;
    
    // Decode command
    m_nRegMod = (nRegCmd & 0x7F);
    m_bRegRWb = (nRegCmd & 0x80) >> 7;

    // Perform range-check
    // TODO: Handle the 16-bit read case
    if (m_nRegAddr >= REG_MAX) {
      // Disable write
      m_nRegAddr = 0;
      m_bRegRWb = 1;
    }
    
  } else if (m_nXferCycle == 1) {
    // DATA cycle

    // Latch data   
    if (m_bRegRWb == 0) {
      // Write command: Latch write data
      m_anRegArray[m_nRegAddr] = nBufDat;
    } else {
      // Read command: nothing to latch
    }
  } else if (m_nXferCycle == 2) {
    // DATA cycle #2
    if (m_bRegRWb == 0) {
      // Write command: Latch write data
      m_anRegArray[m_nRegAddr+1] = nBufDat;
    }
  }

  
  // ----------------------------------
  // Setup output for next cycle
  // ----------------------------------
  m_nXferCycle++;

  if (m_nXferCycle > 3) {
    m_bSpiXferOvf = 1;
  }

  // If we are still the selected slave, then proceed to
  // define our outputs. Otherwise, we can ignore this
  // request.
  if (m_bSlaveSelected) {
    if (m_nXferCycle == 1) {
      // DATA cycle #1
      // We only need to drive SPDR with valid data on a read
      // but for efficiency, always return the current register value
      SPDR = m_anRegArray[m_nRegAddr];
    } else if (m_nXferCycle == 2) {
      // DATA cycle #2
      // For a 16-bit read, we advance the register address
      // TODO: Consider adding array bounds checking here
      SPDR = m_anRegArray[m_nRegAddr+1];
    }
    
  } else {
    // We are not selected for next cycle
    // The pin change int on SSb will be responsible
    // for tristating the MISO pin so that other slaves
    // can respond if they are selected.
  } 

}



void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
