/*
************************************************************************
 ECE 362 - Mini-Project C Source File - Spring 2017
***********************************************************************
	 	   			 		  			 		  		
 Team ID: 30

 Project Name: GameMaster

 Team Members:

   - Team/Doc Leader: Jeff Geiss    Signature: Jeff Geiss
   
   - Software Leader: Kyle Lee      Signature: Kyle Lee

   - Interface Leader: Kevin Du     Signature: Kevin Du

   - Peripheral Leader: Kyle Lee    Signature: Kyle Lee


 Academic Honesty Statement:  In signing above, we hereby certify that we 
 are the individuals who created this HC(S)12 source file and that we have
 not copied the work of any other student (past or present) while completing 
 it. We understand that if we fail to honor this agreement, we will receive 
 a grade of ZERO and be subject to possible disciplinary action.

***********************************************************************

 The objective of this Mini-Project is to implement and track a simplified
 game of Dungeons and Dragons.


***********************************************************************

 List of project-specific success criteria (functionality that will be
 demonstrated):

 1. Display of characters and their stats

 2. Identification of different monster figures

 3. Simulation of the game's required dice-rolling

 4. Health indicator LEDs (brightness/blinking)

 5. Sound effects for damage/victory/defeat
 
 6.	Character selection (at startup)

***********************************************************************

  Date code started: 12/1/17

  Update history (add an entry every time a significant change is made):

  Date: 12/1/17  Name: Kyle Lee   Update: Made skeleton file

  Date: 12/2/17  Name: Kyle Lee   Update: Filled in most of skeleton file

  Date: 12/3/17  Name: Kyle Lee   Update: Finished skeleton file, debugged
  
  Date: 12/5/17  Name: Kyle Lee   Update: More debugging


***********************************************************************
*/

#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */
#include <mc9s12c32.h>

typedef struct character
{
	char type[10];        // Type of character
	int maxhp;            // Max health for the character
	int hp;               // Current health
	int at[3];            // Attack stat (similar to an accuracy stat)
	int ac;               // Armor class stat
	int dmg[3];           // Damage stat (damage done = damage stat + die roll)
	int die[3];           // Max value that can be rolled
	int ini;             // Initiative stat
	int num_attacks;      // Different attacks the character has
	int attack;           // Which attack is next
};

/* All functions after main should be initialized here */
char inchar(void);
void outchar(char x);
void shiftout1(char);	// LCD1 drivers (written previously)
void lcdwait(void);
void send_byte1(char);
void send_i1(char);
void chgline1(char);
void print_c1(char);
void pmsglcd1(char[]);
void shiftout2(char);	// LCD2 drivers (written previously)
void send_byte2(char);
void send_i2(char);
void chgline2(char);
void print_c2(char);
void pmsglcd2(char[]);
void odisp1(int x);   // Display 1 digit numbers (LCD1)
void odisp2(int x);   //                         (LCD2)
void rdisp1(int x);		// Roll display (LCD1) / can be used for 2 digit numbers
void rdisp2(int x);		// Roll display (LCD2)                      
void hpdisp1(int x);   // Display health (LCD1) / can be used for 3 digit numbers
void hpdisp2(int x);   // Display health (LCD2)

void setup(void);				// Start scanning figures
void addString(struct character a, char str[]);
void scan(void);				// Scan current figure
void statdisp1(struct character a);  // Displays monster stats
void statdisp2(struct character a);  // Displays player stats
void addPlayer(int x);			// Type, index
void addMonster(int x);			// Type, index
void gmhpledon(struct character a);             // Turns on GMLED
void gmhpledoff(void);			  // Turns off GMLED
void pchpledon(struct character a);				// Turns on PCLED
void pchpledoff(void);				// Turns off PCLED

void play(void);					// Main function, plays the game
void initiative(void);				// Decide turn order
void pturn(struct character a);             // Player turn
void mturn(struct character a);             // Monster turn
int ptarget(void);				// Turn player chooses a target
int mtarget(void);        // Turn monster chooses a target
int roll1(int x);					// Roll LCD 1; Input max value of "die roll"        
int roll2(int x);					// Roll LCD 2; Input max value of "die roll"
int hit(struct character a, int x);	// Input who took how much damage
void lose(void);					// End game as a loss
void win(void);						// End game as a win
void clearplayers(void);  // Clears player and monster lists when game ends
void clearpb(void);


/* Variable declarations */
struct character players[6];
struct character monsters[10];
int order[16];				// Turn order, randomized in initialize() function
int init[16];         // Values rolled for initative
int temp;             // Temp value for sorting init list
int turn = 0;				// Keeps track of whose turn it is now
int pnum = 0;				// Total # of players
int mnum = 0;				// Total # of monsters
int deadp = 0;				// # of dead players
int deadm = 0;				// # of dead monsters
int tgt;              // Targeted character index
int atk;              // Number that's rolled for an attack
int damage;           // Value to be deducted from health
int dmg;              // Value actually subtracted (to not allow negative hp)
#define charnum pnum + mnum	// Total # of characters (used for setup and turns)
int i = 0;					// Iteration variables i-n
int j = 0;
int k = 0;
int l = 0;
int m = 0;
int n = 0;
int H;                // Hundreds digit
int T;                // Tens digit
int O;                // Ones digit
int random = 0;
int rolled = 0;

int prevpb = 0;
int PBGM1 = 0;
int PBGM2 = 0;
int PBPC1 = 0;
int PBPC2 = 0;
unsigned char holder;

int buffsize = 200;
unsigned char GMLEDBUFF[200];
unsigned char GMBUFF;
unsigned char PCLEDBUFF[200];
unsigned char PCBUFF;

#define GM1 0x20    // PTAD[6]
#define GM2 0x02    // PTAD[1]
#define PC1 0x04    // PTAD[2]
#define PC2 0x08    // PTAD[3]
#define SCAN 0x80   // PTAD[7]
#define GMLED 0x01  // PTT[0]
#define PCLED 0x02  // PTT[1]
#define SS 0x80     // PTT[7]

   	   			 		  			 		       

/* Special ASCII characters */
#define CR 0x0D		// ASCII return 
#define LF 0x0A		// ASCII new line 

/* LCD COMMUNICATION BIT MASKS (note - different than previous labs) */
#define RS2 0x20		// RS pin mask (PTT[5])
#define LCDCLK2 0x40	// LCD EN/CLK pin mask (PTT[6])
#define RS1 0x04		// RS pin mask (PTT[2])
#define LCDCLK1 0x10	// LCD EN/CLK pin mask (PTT[4])
#define RW 0x08       // LCD RW' pinmask(PTT[3])

/* LCD INSTRUCTION CHARACTERS */
#define LCDON 0x0F	// LCD initialization command
#define LCDCLR 0x01	// LCD clear display command
#define TWOLINE 0x38	// LCD 2-line enable command
#define CURMOV 0xFE	// LCD cursor move instruction
#define LINE1 0x80	// LCD line 1 cursor position
#define LINE2 0xC0	// LCD line 2 cursor position

	 	   		
/*	 	   		
***********************************************************************
 Initializations
***********************************************************************
*/

void  initializations(void) {

/* Set the PLL speed (bus clock = 24 MHz) */
  CLKSEL = CLKSEL & 0x80; //; disengage PLL from system
  PLLCTL = PLLCTL | 0x40; //; turn on PLL
  SYNR = 0x02;            //; set PLL multiplier
  REFDV = 0;              //; set PLL divider
  while (!(CRGFLG & 0x08)){  }
  CLKSEL = CLKSEL | 0x80; //; engage PLL

/* Disable watchdog timer (COPCTL register) */
  COPCTL = 0x40   ; //COP off; RTI and COP stopped in BDM-mode

/* Initialize asynchronous serial port (SCI) for 9600 baud, interrupts off initially */
  SCIBDH =  0x00; //set baud rate to 9600
  SCIBDL =  0x9C; //24,000,000 / 16 / 156 = 9600 (approx)  
  SCICR1 =  0x00; //$9C = 156
  SCICR2 =  0x0C; //initialize SCI for program-driven operation
  DDRB   =  0x10; //set PB4 for output mode
  PORTB  =  0x10; //assert DTR pin on COM port
  
// Setup DDRs
  DDRT = 0xFF;
  DDRAD = 0x00;
  ATDDIEN = 0x2E;
  DDRM = 0xFF;
  DDRP = 0xFF;

// Setup PWM
  MODRR = 0x0B;
  PWME = 0x0B;
  PWMPOL = 0x0B;
  PWMPER0 = 0xFF;
  PWMPER1 = 0xFF;
  PWMPER3 = 0xFF;
  PWMDTY0 = 0x00;
  PWMDTY1 = 0x00;
  PWMDTY3 = 0x00;
  PWMPRCLK = 0x02;
  PWMSCLA = 1;
  PWMSCLB = 0x40;
  PWMCTL = 0;
  PWMCAE = 0;
  PWMCLK = 0x08;

/* Initialize peripherals */

// Setup ATD channel 7 for scanninng figures
  ATDCTL2 = 0x80;
  ATDCTL3 = 0x10;
  ATDCTL4 = 0x85;
  
// Setup Timer interrupt for 20 ms
  TSCR1 = 0x80;
  TSCR2 = 0x0E;
  TC7   = 7500;
  TIOS  = 0x80;
  TIE   = 0x80; 
  
// Setup RTI for 2.048 ms
  CRGINT = 0x80;
  RTICTL = 0x27;
  
// Setup SPI for 6Mbs
  SPICR1 =  0x50;
  SPICR2 =  0x00;
  SPIBR  =  0x01;
  
// LCD Initialization
  PTT = 0;
  PTT |= LCDCLK1;
  PTT &= ~RS1;
  PTT |= LCDCLK2;
  PTT &= ~RS2;
  PTT &= ~RW;
  send_i1(LCDON);
  send_i1(TWOLINE);
  send_i1(LCDCLR);
  send_i2(LCDON);
  send_i2(TWOLINE);
  send_i2(LCDCLR);
  chgline1(LINE1);
  pmsglcd1("Hello");
  chgline2(LINE1);
  pmsglcd2("Hello");
  lcdwait();
	      
	      
}

	 		  			 		  		
/*	 		  			 		  		
***********************************************************************
Main
***********************************************************************
*/
void main(void) {
  	DisableInterrupts
  TIOS  = 0x80;
	initializations(); 		  			 		  		
	EnableInterrupts;


  
 for(;;) {
  
/* < start of your main loop > */ 
  setup();
  play();
  

  
   } /* loop forever */
   
}   /* do not leave main */


/*
***********************************************************************
***********************************************************************
    TEMPORARY SETUP FUNCTION (PROTOTYPING ONLY)
***********************************************************************
***********************************************************************
void setup(void){
  send_i1(LCDCLR);
  chgline1(LINE1);
  pmsglcd1("Starting up");
  send_i2(LCDCLR);
  chgline2(LINE1);
  pmsglcd2("Starting up");
  for(pnum = 0; pnum < 3; pnum++){
    players[pnum].maxhp = 50;
    players[pnum].hp = 50;
    players[pnum].ac = 14;
    players[pnum].at[0] = 5;
    players[pnum].dmg[0] = 2;
    players[pnum].die[0] = 8;
    players[pnum].at[1] = 0;
    players[pnum].dmg[1] = 0;
    players[pnum].die[1] = 0; 
    players[pnum].at[2] = 0;
    players[pnum].dmg[2] = 0;
    players[pnum].die[2] = 0;
    players[pnum].num_attacks = 1;
    players[pnum].attack = 0;
    players[pnum].ini = 2;
  }
  for(mnum = 0; mnum < 3; mnum++){
    monsters[mnum].maxhp = 50;
    monsters[mnum].hp = 50;
    monsters[mnum].ac = 14;
    monsters[mnum].at[0] = 5;
    monsters[mnum].dmg[0] = 2;
    monsters[mnum].die[0] = 8;
    monsters[mnum].at[1] = 0;
    monsters[mnum].dmg[1] = 0;
    monsters[mnum].die[1] = 0;
    monsters[mnum].at[2] = 0;
    monsters[mnum].dmg[2] = 0;
    monsters[mnum].die[2] = 0;
    monsters[mnum].num_attacks = 1;
    monsters[mnum].attack = 0;
    monsters[mnum].ini = 2;
  }
  players[0].type[0] = 'R';
  players[0].type[1] = 'o';
  players[0].type[2] = 'g';
  players[0].type[3] = 'u';
  players[0].type[4] = 'e';
  players[1].type[0] = 'F';
  players[1].type[1] = 'i';
  players[1].type[2] = 'g';
  players[1].type[3] = 'h';
  players[1].type[4] = 't';
  players[1].type[5] = 'e';
  players[1].type[6] = 'r';
  players[2].type[0] = 'R'; 
  players[2].type[1] = 'a';
  players[2].type[2] = 'n';
  players[2].type[3] = 'g';
  players[2].type[4] = 'e';
  players[2].type[5] = 'r';
  monsters[0].type[0] = 'Z';
  monsters[0].type[1] = 'o';
  monsters[0].type[2] = 'm';
  monsters[0].type[3] = 'b';
  monsters[0].type[4] = 'i';
  monsters[0].type[5] = 'e';
  monsters[1].type[0] = 'S';
  monsters[1].type[1] = 'k';
  monsters[1].type[2] = 'e';
  monsters[1].type[3] = 'l';
  monsters[1].type[4] = 'e';
  monsters[1].type[5] = 't';
  monsters[1].type[6] = 'o';
  monsters[1].type[7] = 'n';
  monsters[2].type[0] = 'V';
  monsters[2].type[1] = 'a';
  monsters[2].type[2] = 'm';
  monsters[2].type[3] = 'p';
  monsters[2].type[4] = 'i';
  monsters[2].type[5] = 'r';
  monsters[2].type[6] = 'e';
}
***********************************************************************
***********************************************************************
    END OF TEMPORARY SETUP FUNCTION
***********************************************************************
***********************************************************************
*/

void setup(void){
	//print scanning
	send_i2(LCDCLR);
	chgline2(LINE1);
	pmsglcd2("GM is scanning");
	chgline2(LINE2);
	pmsglcd2("figures");
	send_i1(LCDCLR);
	chgline1(LINE1);
	pmsglcd1("To Scan Press L");
	chgline1(LINE2);
	pmsglcd1("To Start Press R");
  while(!pnum | !mnum){
  	while(!PBGM2){
  		if(PBGM1){
  			clearpb();
  			scan();
      	send_i2(LCDCLR);
      	chgline2(LINE1);
      	pmsglcd2("GM is scanning");
      	chgline2(LINE2);
        pmsglcd2("figures");
      	send_i1(LCDCLR);
      	chgline1(LINE1);
      	pmsglcd1("To Scan Press L");
      	chgline1(LINE2);
      	pmsglcd1("To Start Press R");
  		}
  	}
  	if(!pnum | !mnum){  
    	send_i2(LCDCLR);
    	chgline2(LINE1);
    	pmsglcd2("Invalid game");
    	chgline2(LINE2);
      pmsglcd2("Scan more figures");
    	send_i1(LCDCLR);
    	chgline1(LINE1);
    	pmsglcd1("Invalid game");
    	chgline1(LINE2);
    	pmsglcd1("Add figures");
      while(!(PBGM1 | PBGM2 | PBPC1 | PBPC2));
      clearpb();
  	}
  	  
  }
	clearpb();
}
void scan(void){
		ATDCTL5 = 0x00;
		while(!ATDSTAT0_SCF);
		holder = ATDDR0H;
		if(holder < 0x04){
		  send_i1(LCDCLR);
		  chgline1(LINE1);
		  pmsglcd1("No figure");
		  chgline1(LINE2);
		  pmsglcd1("detected");
      while(!(PBGM1 | PBGM2 | PBPC1 | PBPC2));
      clearpb();
      return;
		} 
		if(holder > 0x80){
			if(mnum == 9){
				//print too many monsters
    	  send_i1(LCDCLR);
				chgline1(LINE1);
				pmsglcd1("Too Many Monsters");
        while(!(PBGM1 | PBGM2 | PBPC1 | PBPC2));
        clearpb();
				return;
			}
			if(holder > 0xF0){
				addMonster(1);
				//insert more monster stats
			}
			else if(holder > 0xEA){
				addMonster(2);
			}
			else if(holder > 0xE4){
				addMonster(3);
			}
			else if(holder > 0xD8){
				addMonster(4);
			}
			else if(holder > 0xC8){
				addMonster(5);
			}
			else if(holder > 0xB8){
				addMonster(6);
			}
			else if(holder > 0xA9){
				addMonster(7);
			}
			else if(holder > 0x9F){
				addMonster(8);
			}
			else if(holder > 0x92){
				addMonster(9);
			}else{
				addMonster(10);
			}
				
		}//monster
		else{
			if(pnum == 5){
				//print invalid
				send_i1(LCDCLR);
				chgline1(LINE1);
				pmsglcd1("Too Many Characters"); 
        while(!(PBGM1 | PBGM2 | PBPC1 | PBPC2));
        clearpb();
				return;
			}
			if(holder < 0x1E){
				addPlayer(1);
			}
			else if(holder < 0x25){
				addPlayer(2);
			}
			else if(holder < 0x2A){
				addPlayer(3);
			}
			else if(holder < 0x34){
				addPlayer(4);
			}
			else if(holder < 0x46){
				addPlayer(5);
			}
			else if(holder < 0x70){
				addPlayer(6);
			}
		}// player character
	
}
void addMonster(int x){
	switch(x){
		case 1:
    monsters[mnum].type[0] = 'S';
    monsters[mnum].type[1] = 'k';
    monsters[mnum].type[2] = 'e';
    monsters[mnum].type[3] = 'l';
    monsters[mnum].type[4] = 'e';
    monsters[mnum].type[5] = 't';
    monsters[mnum].type[6] = 'o';
    monsters[mnum].type[7] = 'n';
		monsters[mnum].maxhp = 13;
		monsters[mnum].hp = 13;
		monsters[mnum].ac = 13;
		monsters[mnum].at[0] = 4;
		monsters[mnum].dmg[0] = 2;
		monsters[mnum].die[0] = 6;
		monsters[mnum].ini = 1;
		monsters[mnum].num_attacks = 1;		
		break;
		case 2:
//		addString(monsters[mnum],"Giant");
    monsters[mnum].type[0] = 'G';
    monsters[mnum].type[1] = 'i';
    monsters[mnum].type[2] = 'a';
    monsters[mnum].type[3] = 'n';
    monsters[mnum].type[4] = 't';
		monsters[mnum].maxhp = 105;
		monsters[mnum].hp = 105;
		monsters[mnum].ac = 13;
		monsters[mnum].at[0] = 8;
		monsters[mnum].dmg[0] = 9;
		monsters[mnum].die[0] = 16;
		monsters[mnum].at[1] = 6;
		monsters[mnum].dmg[1] = 0;
		monsters[mnum].die[1] = 24;
		monsters[mnum].ini = 0;
		monsters[mnum].num_attacks = 2;
		break;
		case 3:
//		addString(monsters[mnum],"Yuan-Ti");
    monsters[mnum].type[0] = 'Y';
    monsters[mnum].type[1] = 'u';
    monsters[mnum].type[2] = 'a';
    monsters[mnum].type[3] = 'n';
    monsters[mnum].type[4] = '-';
    monsters[mnum].type[5] = 'T';
    monsters[mnum].type[6] = 'i';
		monsters[mnum].maxhp = 127;
		monsters[mnum].hp = 127;
		monsters[mnum].ac = 15;
		monsters[mnum].at[0] = 7;
		monsters[mnum].dmg[0] = 4;
		monsters[mnum].die[0] = 20;
		monsters[mnum].at[1] = 8;
		monsters[mnum].dmg[1] = 7;
		monsters[mnum].die[1] = 6;
		monsters[mnum].at[2] = 7;
		monsters[mnum].dmg[2] = 7;
		monsters[mnum].die[2] = 6;
		monsters[mnum].ini = 4;
		monsters[mnum].num_attacks = 3;
		break;
		case 4:
//		addString(monsters[mnum],"Bulette");
    monsters[mnum].type[0] = 'B';
    monsters[mnum].type[1] = 'u';
    monsters[mnum].type[2] = 'l';
    monsters[mnum].type[3] = 'e';
    monsters[mnum].type[4] = 't';
    monsters[mnum].type[5] = 't';
    monsters[mnum].type[6] = 'e';
		monsters[mnum].maxhp = 105;
		monsters[mnum].hp = 105;
		monsters[mnum].ac = 13;
		monsters[mnum].at[0] = 5;
		monsters[mnum].dmg[0] = 16;
		monsters[mnum].die[0] = 24;
		monsters[mnum].at[1] = 6;
		monsters[mnum].dmg[1] = 4;
		monsters[mnum].die[1] = 12;
		monsters[mnum].ini = 4;
		monsters[mnum].num_attacks = 2;
		break;
		case 5:
//		addString(monsters[mnum],"Owlbear"); 
    monsters[mnum].type[0] = 'O';
    monsters[mnum].type[1] = 'w';
    monsters[mnum].type[2] = 'l';
    monsters[mnum].type[3] = 'b';
    monsters[mnum].type[4] = 'e';
    monsters[mnum].type[5] = 'a';
    monsters[mnum].type[6] = 'r';
		monsters[mnum].maxhp = 59;
		monsters[mnum].hp = 59;
		monsters[mnum].ac = 13;
		monsters[mnum].at[0] = 9;
		monsters[mnum].dmg[0] = 5;
		monsters[mnum].die[0] = 10;
		monsters[mnum].at[1] = 8;
		monsters[mnum].dmg[1] = 5;
		monsters[mnum].die[1] = 8;
		monsters[mnum].at[2] = 8;
		monsters[mnum].dmg[2] = 5;
		monsters[mnum].die[2] = 8;
		monsters[mnum].ini = 2;
		monsters[mnum].num_attacks = 3;
		break;
		case 6:
//		addString(monsters[mnum],"Zombie"); 
    monsters[mnum].type[0] = 'Z';
    monsters[mnum].type[1] = 'o';
    monsters[mnum].type[2] = 'm';
    monsters[mnum].type[3] = 'b';
    monsters[mnum].type[4] = 'i';
    monsters[mnum].type[5] = 'e';
		monsters[mnum].maxhp = 22;
		monsters[mnum].hp = 22;
		monsters[mnum].ac = 8;
		monsters[mnum].at[0] = 3;
		monsters[mnum].dmg[0] = 1;
		monsters[mnum].die[0] = 6;
		monsters[mnum].ini = 0;
		monsters[mnum].num_attacks = 1;
		break;
		case 7:
//		addString(monsters[mnum],"Dragon"); 
    monsters[mnum].type[0] = 'D';
    monsters[mnum].type[1] = 'r';
    monsters[mnum].type[2] = 'a';
    monsters[mnum].type[3] = 'g';
    monsters[mnum].type[4] = 'o';
    monsters[mnum].type[5] = 'n';
		monsters[mnum].maxhp = 195;
		monsters[mnum].hp = 195;
		monsters[mnum].ac = 19;
		monsters[mnum].at[0] = 12;
		monsters[mnum].dmg[0] = 11;
		monsters[mnum].die[0] = 10;
		monsters[mnum].at[1] = 11;
		monsters[mnum].dmg[1] = 9;
		monsters[mnum].die[1] = 6;
		monsters[mnum].at[2] = 11;
		monsters[mnum].dmg[2] = 9;
		monsters[mnum].die[2] = 8;
		monsters[mnum].ini = 2;
		monsters[mnum].num_attacks = 3;
		break;
		case 8: 
//		addString(monsters[mnum],"Werebear");  
    monsters[mnum].type[0] = 'W';
    monsters[mnum].type[1] = 'e';
    monsters[mnum].type[2] = 'r';
    monsters[mnum].type[3] = 'e';
    monsters[mnum].type[4] = 'b';
    monsters[mnum].type[5] = 'e';
    monsters[mnum].type[6] = 'a';
    monsters[mnum].type[7] = 'r';
		monsters[mnum].maxhp = 135;
		monsters[mnum].hp = 135;
		monsters[mnum].ac = 10;
		monsters[mnum].at[0] = 7;
		monsters[mnum].dmg[0] = 4;
		monsters[mnum].die[0] = 12;
		monsters[mnum].at[1] = 5;
		monsters[mnum].dmg[1] = 2;
		monsters[mnum].die[1] = 8;
		monsters[mnum].ini = 2;
		monsters[mnum].num_attacks = 2;
		break;
		case 9:
//		addString(monsters[mnum],"Dire Wolf");
    monsters[mnum].type[0] = 'D';
    monsters[mnum].type[1] = 'i';
    monsters[mnum].type[2] = 'r';
    monsters[mnum].type[3] = 'e';
    monsters[mnum].type[4] = ' ';
    monsters[mnum].type[5] = 'W';
    monsters[mnum].type[6] = 'o';
    monsters[mnum].type[7] = 'l';
    monsters[mnum].type[8] = 'f';
		monsters[mnum].maxhp = 37;
		monsters[mnum].hp = 37;
		monsters[mnum].ac = 14;
		monsters[mnum].at[0] = 5;
		monsters[mnum].dmg[0] = 6;
		monsters[mnum].die[0] = 6;
		monsters[mnum].ini = 2;
		monsters[mnum].num_attacks = 1;
		break;
		case 10:
//		addString(monsters[mnum],"Lava Giant");
    monsters[mnum].type[0] = 'L';
    monsters[mnum].type[1] = 'a';
    monsters[mnum].type[2] = 'v';
    monsters[mnum].type[3] = 'a';
    monsters[mnum].type[4] = ' ';
    monsters[mnum].type[5] = 'G';
    monsters[mnum].type[6] = 'i';
    monsters[mnum].type[7] = 'a';
    monsters[mnum].type[8] = 'n';
    monsters[mnum].type[9] = 't';
		monsters[mnum].maxhp = 102;
		monsters[mnum].hp = 102;
		monsters[mnum].ac = 13;
		monsters[mnum].at[0] = 6;
		monsters[mnum].dmg[0] = 6;
		monsters[mnum].die[0] = 6;
		monsters[mnum].at[1] = 6;
		monsters[mnum].dmg[1] = 6;
		monsters[mnum].die[1] = 6;
		monsters[mnum].ini = 5;
		monsters[mnum].num_attacks = 2;
		break;
	}
	statdisp1(monsters[mnum]);
	while(!PBGM1);
	clearpb();
	mnum++;
}
void addPlayer(int x){
	switch(x){
		case 1:
//		addString(players[pnum],"Ranger");
    players[pnum].type[0] = 'R';
    players[pnum].type[1] = 'a';
    players[pnum].type[2] = 'n';
    players[pnum].type[3] = 'g';
    players[pnum].type[4] = 'e';
    players[pnum].type[5] = 'r';
		players[pnum].maxhp = 54;
		players[pnum].hp = 54;
		players[pnum].ac = 13;
		players[pnum].at[0] = 7;
		players[pnum].dmg[0] = 3;
		players[pnum].die[0] = 6;
		players[pnum].at[1] = 4;
		players[pnum].dmg[1] = 3;
		players[pnum].die[1] = 8;
		players[pnum].ini = 1;
		players[pnum].num_attacks = 2;	
		break;
		case 2:
//		addString(players[pnum],"Barbarian");
    players[pnum].type[0] = 'B';
    players[pnum].type[1] = 'a';
    players[pnum].type[2] = 'r';
    players[pnum].type[3] = 'b';
    players[pnum].type[4] = 'a';
    players[pnum].type[5] = 'r';
    players[pnum].type[6] = 'i';
    players[pnum].type[7] = 'a';
    players[pnum].type[8] = 'n';
		players[pnum].maxhp = 115;
		players[pnum].hp = 115;
		players[pnum].ac = 11;
		players[pnum].at[0] = 7;
		players[pnum].dmg[0] = 2;
		players[pnum].die[0] = 12;
		players[pnum].at[1] = 5;
		players[pnum].dmg[1] = 4;
		players[pnum].die[1] = 12;
		players[pnum].ini = 3;
		players[pnum].num_attacks = 2;
		break;
		case 3:
//		addString(players[pnum],"Paladin"); 
    players[pnum].type[0] = 'P';
    players[pnum].type[1] = 'a';
    players[pnum].type[2] = 'l';
    players[pnum].type[3] = 'a';
    players[pnum].type[4] = 'd';
    players[pnum].type[5] = 'i';
    players[pnum].type[6] = 'n';
		players[pnum].maxhp = 103;
		players[pnum].hp = 103;
		players[pnum].ac = 17;
		players[pnum].at[0] = 6;
		players[pnum].dmg[0] = 2;
		players[pnum].die[0] = 8;
		players[pnum].at[1] = 5;
		players[pnum].dmg[1] = 2;
		players[pnum].die[1] = 8;
		players[pnum].ini = 1;
		players[pnum].num_attacks = 2;
		break;
		case 4:
//		addString(players[pnum],"Rogue"); 
    players[pnum].type[0] = 'R';
    players[pnum].type[1] = 'o';
    players[pnum].type[2] = 'g';
    players[pnum].type[3] = 'u';
    players[pnum].type[4] = 'e';
		players[pnum].maxhp = 42;
		players[pnum].hp = 42;
		players[pnum].ac = 14;
		players[pnum].at[0] = 9;
		players[pnum].dmg[0] = 2;
		players[pnum].die[0] = 8;
		players[pnum].at[1] = 6;
		players[pnum].dmg[1] = 1;
		players[pnum].die[1] = 6;
		players[pnum].at[2] = 5;
		players[pnum].dmg[2] = 1;
		players[pnum].die[2] = 6;
		players[pnum].ini = 5;
		players[pnum].num_attacks = 3;
		break;
		case 5:
//		addString(players[pnum],"Cleric");
    players[pnum].type[0] = 'C';
    players[pnum].type[1] = 'l';
    players[pnum].type[2] = 'e';
    players[pnum].type[3] = 'r';
    players[pnum].type[4] = 'i';
    players[pnum].type[5] = 'c';
		players[pnum].maxhp = 31;
		players[pnum].hp = 31;
		players[pnum].ac = 12;
		players[pnum].at[0] = 5;
		players[pnum].dmg[0] = 1;
		players[pnum].die[0] = 6;
		players[pnum].ini = 2;
		players[pnum].num_attacks = 1;
		break;
		case 6:
//		addString(players[pnum],"Wizard");
    players[pnum].type[0] = 'W';
    players[pnum].type[1] = 'i';
    players[pnum].type[2] = 'z';
    players[pnum].type[3] = 'a';
    players[pnum].type[4] = 'r';
    players[pnum].type[5] = 'd';
		players[pnum].maxhp = 25;
		players[pnum].hp = 25;
		players[pnum].ac = 10;
		players[pnum].at[0] = 3;
		players[pnum].dmg[0] = 5;
		players[pnum].die[0] = 10;
		players[pnum].ini = 1;
		players[pnum].num_attacks = 1;
		break;
	}
	statdisp1(players[pnum]);
	while(!PBGM1);
	PBGM1 = 0;
	pnum++;
}
void addString(struct character a, char* str){
	for(i = 0; *(str + i) != 0;i++){
		a.type[i] = *(str + i);
	}
}



void play(void){
  initiative();
  send_i1(LCDCLR);
  send_i2(LCDCLR);
  turn = 0;
  while(1){
    if(deadp == pnum){
      lose();
      break;
    }
    if(deadm == mnum){
      win();
      break;
    }
    if(order[turn] < pnum){
      if(players[order[turn]].hp > 0){
        pturn(players[order[turn]]);
      }
    } else{
      if(monsters[order[turn] - pnum].hp > 0){
        mturn(monsters[order[turn] - pnum]);
      }
    }
    turn++;
    turn %= (charnum);
  }
}

void initiative(void){
  for(n =0; n < charnum; n++){
    order[n] = n;
  }
  for(n = 0; n < pnum; n++){
    send_i1(LCDCLR);
    chgline1(LINE1);
    pmsglcd1(players[n].type);
    pmsglcd1(" rolling");
    chgline1(LINE2);
    pmsglcd1("initiative");
    send_i2(LCDCLR);
    chgline2(LINE1);
    pmsglcd2(players[n].type);
    pmsglcd2(" init");
    chgline2(LINE2);
    pmsglcd2("Press L to roll");
    while(!PBPC1);
    clearpb();
    init[n] = roll2(20) + players[n].ini;
  }
  for(n = 0; n < mnum; n++){ 
    send_i2(LCDCLR);
    chgline2(LINE1);
    pmsglcd2(monsters[n].type);
    pmsglcd2(" rolling");
    chgline2(LINE2);
    pmsglcd2("initiative");
    send_i1(LCDCLR);
    chgline1(LINE1);
    pmsglcd1(monsters[n].type);
    pmsglcd1(" init");
    chgline1(LINE2);
    pmsglcd1("Press L to roll");
    while(!PBGM1);
    clearpb();
    init[n + pnum] = roll1(20) + monsters[n].ini;
  }
  for(n = 0; n < charnum - 1; n++){
    for(m = (n + 1); m < charnum; m++){
      if(init[n] < init[m]){
        temp = init[n];
        init[n] = init[m];
        init[m] = temp;
        temp = order[n];
        order[n] = order[m];
        order[m] = temp;
      }
    }
  }
}
  

void pturn(struct character a){
  statdisp2(a);
  send_i1(LCDCLR);
  chgline1(LINE1);
  pmsglcd1(a.type);
  pmsglcd1("'s turn");
  chgline1(LINE2);
  pmsglcd1("Please wait");
  pchpledon(a);
  while(!PBPC1);
  clearpb();
  for(n = 0; n < a.num_attacks; n++){
    send_i2(LCDCLR);
    chgline2(LINE1);
    pmsglcd2("Choose a target");
    chgline2(LINE2);
    pmsglcd2("Press L");
    while(!PBPC1);
    clearpb();
    tgt = ptarget();
    if(tgt >= mnum){
      continue;
    }
    send_i2(LCDCLR);
    chgline2(LINE1);
    pmsglcd2("Roll attack");
    chgline2(LINE2);
    pmsglcd2("Press L");
    while(!PBPC1);
    clearpb();
    atk = roll2(20);
    if(a.at[n] + atk >= monsters[tgt].ac){
      send_i2(LCDCLR);
      chgline2(LINE1);
      pmsglcd2("Roll damage");
      chgline2(LINE2);
      pmsglcd2("Press L");
      while(!PBPC1);
      clearpb();
      damage = a.dmg[n] + roll2(a.die[n]);
      dmg = hit(monsters[tgt], damage);
      monsters[tgt].hp -= dmg;
      if(monsters[tgt].hp == 0){
        deadm++;
        send_i1(LCDCLR);
        chgline1(LINE1);
        pmsglcd1(monsters[tgt].type);
        pmsglcd1(" was");
        chgline1(LINE2);
        pmsglcd1("defeated!");
        send_i2(LCDCLR);
        chgline2(LINE1);
        pmsglcd2(monsters[tgt].type);
        pmsglcd2(" was");
        chgline2(LINE2);
        pmsglcd2("defeated!");
        while(!(PBGM1 | PBPC1));
        clearpb();
      }
    } else{
      send_i2(LCDCLR);
      chgline2(LINE1);
      pmsglcd2("Attack failed");
      chgline2(LINE2);
      pmsglcd2("Press L");
      while(!PBPC1);
      clearpb();
      send_i2(LCDCLR);
    }
  }
  pchpledoff();
}

void mturn(struct character a){  
  statdisp1(a);
  send_i2(LCDCLR);
  chgline2(LINE1);
  pmsglcd2(a.type);
  pmsglcd2("'s turn");
  chgline2(LINE2);
  pmsglcd2("Please wait");
  gmhpledon(a);
  while(!PBGM1);
  clearpb();
  for(n = 0; n < a.num_attacks; n++){
    send_i1(LCDCLR);
    chgline1(LINE1);
    pmsglcd1("Choose a target");
    chgline1(LINE2);
    pmsglcd1("Press L");
    while(!PBGM1);
    clearpb();
    tgt = mtarget();
    if(tgt >= pnum){
      continue;
    }
    send_i1(LCDCLR);
    chgline1(LINE1);
    pmsglcd1("Roll attack");
    chgline1(LINE2);
    pmsglcd1("Press L");
    while(!PBGM1);
    clearpb();
    atk = roll1(20);
    if(a.at[n] + atk >= players[tgt].ac){
      send_i1(LCDCLR);
      chgline1(LINE1);
      pmsglcd1("Roll damage");
      chgline1(LINE2);
      pmsglcd1("Press L");
      while(!PBGM1);
      clearpb();
      damage = a.dmg[n] + roll1(a.die[n]);
      dmg = hit(players[tgt], damage);
      players[tgt].hp -= dmg;
      if(players[tgt].hp == 0){
        deadp++;
        send_i1(LCDCLR);
        chgline1(LINE1);
        pmsglcd1(players[tgt].type);
        pmsglcd1(" was");
        chgline1(LINE2);
        pmsglcd1("defeated!");
        send_i2(LCDCLR);
        chgline2(LINE1);
        pmsglcd2(players[tgt].type);
        pmsglcd2(" was");
        chgline2(LINE2);
        pmsglcd2("defeated!");
        while(!(PBGM1 | PBPC1));
        clearpb();
      }
    } else{
      send_i1(LCDCLR);
      chgline1(LINE1);
      pmsglcd1("Attack failed!");
      chgline1(LINE2);
      pmsglcd1("Press L");
      while(!PBGM1);
      clearpb();
      send_i1(LCDCLR);
    }
  }
  gmhpledoff();
}

int ptarget(void){
  l = 0;
  send_i2(LCDCLR);
  odisp2(l + 1);
  pmsglcd2(": ");
  pmsglcd2(monsters[l].type);
  while(!PBPC2){
    if((l < mnum) && (monsters[l].hp == 0)){
      l = (l + 1) % (mnum + 1); 
      send_i2(LCDCLR);
      chgline2(LINE1);
      if(l >= mnum){
        odisp2(l + 1);
        pmsglcd2(": Pass Turn");
      }
      if(l < mnum){
        odisp2(l + 1);
        pmsglcd2(": ");
        pmsglcd2(monsters[l].type);
      }
    }
    if(PBPC1){
      clearpb();
      l = (l + 1) % (mnum + 1);
      send_i2(LCDCLR);
      chgline2(LINE1);
      if(l >= mnum){ 
        odisp2(l + 1);
        pmsglcd2(": Pass Turn");
      }
      if(l < mnum){
        odisp2(l + 1);
        pmsglcd2(": ");
        pmsglcd2(monsters[l].type);
      }
    }
  }
  clearpb();
  return l;
}

int mtarget(void){
  l = 0;
  send_i1(LCDCLR);
  odisp1(l + 1);
  pmsglcd1(": ");
  pmsglcd1(players[l].type);
  while(!PBGM2){
    if((l < pnum) && (players[l].hp == 0)){
      l = (l + 1) % (pnum + 1); 
      send_i1(LCDCLR);
      chgline1(LINE1);
      if(l >= pnum){ 
        odisp1(l + 1);
        pmsglcd1(": Pass Turn");
      }
      if(l < pnum){
        odisp1(l + 1);
        pmsglcd1(": ");
        pmsglcd1(players[l].type);
      }
    }
    if(PBGM1){
      clearpb();
      l = (l + 1) % (pnum + 1); 
      send_i1(LCDCLR);
      chgline1(LINE1);
      if(l >= pnum){ 
        odisp1(l + 1);
        pmsglcd1(": Pass Turn");
      }
      if(l < pnum){
        odisp1(l + 1);
        pmsglcd1(": ");
        pmsglcd1(players[l].type);
      }
    } 
  }
  clearpb();
  return l;
}

int hit(struct character a, int x){
  if(x > a.hp){
    dmg = a.hp;
  } else{
    dmg = x;
  }
  send_i1(LCDCLR);
  chgline1(LINE1);
  pmsglcd1(a.type);
  pmsglcd1(" took");
  chgline1(LINE2);
  rdisp1(x);
  pmsglcd1(" damage!");
  send_i2(LCDCLR);
  chgline2(LINE1);
  pmsglcd2(a.type);
  pmsglcd2(" took");
  chgline2(LINE2);
  rdisp2(x);
  pmsglcd2(" damage!");
  PWMDTY3 = PWMPER3 / 2;
  for(k = 0; k < 1500; k++){
    lcdwait();
  }
  PWMDTY3 = 0;
  while(!(PBGM1 | PBPC1));
  clearpb();
  send_i1(LCDCLR);
  send_i2(LCDCLR);
  return dmg;
}

/*
***********************************************************************                       
 RTI interrupt service routine: RTI_ISR
************************************************************************
*/

interrupt 7 void RTI_ISR(void)
{
  	// clear RTI interrupt flagt 
  	CRGFLG = CRGFLG | 0x80;
    random = (random + 1) % 1000; 
    if(!(PTAD & GM1)){
	    if(!(prevpb & GM1)){
	      PBGM1++;
	    }
	    prevpb |= 0x20;
	  } else{
	    prevpb &= 0x0E;
	  }
	  if(!(PTAD & GM2)){
	    if(!(prevpb & GM2)){
	      PBGM2++;
	    }
	    prevpb|= 0x02;
	  } else{
	    prevpb &= 0x2C;
	  }
    if(!(PTAD & PC1)){
	    if(!(prevpb & PC1)){
	      PBPC1++;
	    }
	    prevpb |= 0x04;
	  } else{
	    prevpb &= 0x2A;
	  }
	  if(!(PTAD & PC2)){
	    if(!(prevpb & PC2)){
	      PBPC2++;
	    prevpb |= 0x08;
	    }
	  } else{
	    prevpb &= 0x26;
	  }

}

/*
***********************************************************************                       
  TIM interrupt service routine	  		
***********************************************************************
*/

interrupt 15 void TIM_ISR(void)
{
  	// clear TIM CH 7 interrupt flag 
 	TFLG1 = TFLG1 | 0x80; 
  PWMDTY0 = GMLEDBUFF[GMBUFF];
  PWMDTY1 = PCLEDBUFF[PCBUFF];
  GMBUFF = (GMBUFF + 1) % buffsize;
  PCBUFF = (PCBUFF + 1) % buffsize;
}


/*
***********************************************************************
  LED Functions (GM/PC on/off)
  Modify the circular buffer
***********************************************************************
*/
void gmhpledon(struct character a)
{
	for(n = 0; n < a.hp * 100 / a.maxhp; n++)
	{
		GMLEDBUFF[n] = (n * a.maxhp / a.hp) * PWMPER0 / 100;
	}
	for(n = a.hp * 100 / a.maxhp; n < a.hp * 200 / a.maxhp; n++)
	{
		GMLEDBUFF[n] = (200 - (n * a.maxhp / a.hp)) * PWMPER0 / 100;
	}
}

void gmhpledoff(void)
{
	for(n = 0; n < buffsize; n++)
	{
		GMLEDBUFF[n] = 0;
	}
}

void pchpledon(struct character a)
{  
	for(n = 0; n < a.hp * 100 / a.maxhp; n++)
	{
		PCLEDBUFF[n] = (n * a.maxhp / a.hp) * PWMPER1 / 100;
	}
	for(n = a.hp * 100 / a.maxhp; n < a.hp * 200 / a.maxhp; n++)
	{
		PCLEDBUFF[n] = (200 - (n * a.maxhp / a.hp)) * PWMPER1 / 100;
	}
}

void pchpledoff(void)
{
	for(n = 0; n < buffsize; n++)
	{
		PCLEDBUFF[n] = 0;
	}
}

/*
***********************************************************************                       
  Roll function	LCD1
***********************************************************************
*/

int roll1(int x){
  while(!PBGM1){
    for(k = 0; k < 50; k++){
      lcdwait();
    }
    rolled = random % x + 1;
    send_i1(LCDCLR);
    chgline1(LINE1);
    pmsglcd1("Rolling...");
    chgline1(LINE2);
    rdisp1(rolled);
    pmsglcd1(" ");
  }
  clearpb();
  for(k = 50; k < 100; k += 3){
    for(l = 0; l < k; l++){
      lcdwait();
    }
    rolled = random % x + 1;
    send_i1(LCDCLR);
    chgline1(LINE1);
    pmsglcd1("Rolling...");
    chgline1(LINE2);
    rdisp1(rolled); 
  }
  for(k = 0; k < 1500; k++){
    lcdwait();
  }
  clearpb();
  return rolled;
}

/*
***********************************************************************                       
  Roll function	LCD2	
***********************************************************************
*/

int roll2(int x){
  while(!PBPC1){
    for(k = 0; k < 50; k++){
      lcdwait();
    }
    rolled = random % x + 1;
    send_i2(LCDCLR);
    chgline2(LINE1);
    pmsglcd2("Rolling...");
    chgline2(LINE2);
    rdisp2(rolled);
  } 
  clearpb();
  for(k = 50; k < 100; k += 3){
    for(l = 0; l < k; l++){
      lcdwait();
    }
    rolled = random % x + 1; 
    send_i2(LCDCLR);
    chgline2(LINE1);
    pmsglcd2("Rolling...");
    chgline2(LINE2);
    rdisp2(rolled); 
  }
  for(k = 0; k < 1500; k++){
    lcdwait();
  }
  clearpb();
  return rolled;
}

/*
***********************************************************************                       
  SCI interrupt service routine		 		  		
***********************************************************************
*/

interrupt 20 void SCI_ISR(void)
{
 


}

void statdisp1(struct character a){
	send_i1(LCDCLR);
	chgline1(LINE1);
	pmsglcd1(a.type);
	pmsglcd1(" HP:");
	hpdisp1(a.hp);
	chgline1(LINE2);
	pmsglcd1("AC:");
	rdisp1(a.ac);
	print_c1(0xD2);
	odisp1(a.attack + 1);
	pmsglcd1(":");
	rdisp1(a.at[a.attack]);
	pmsglcd1("D:d");
	rdisp1(a.die[a.attack]);
	pmsglcd1("+");
	odisp1(a.dmg[a.attack]);	
}
void statdisp2(struct character a){
	send_i2(LCDCLR);
	chgline2(LINE1);
	pmsglcd2(a.type);
	pmsglcd2(" HP:");
	hpdisp2(a.hp);
	chgline2(LINE2);
	pmsglcd2("AC:");
	rdisp2(a.ac);
	print_c2(0xD2);
	odisp2(a.attack + 1);
	pmsglcd2(":");
	rdisp2(a.at[a.attack]);
	pmsglcd2("D:d");
	rdisp2(a.die[a.attack]);
	pmsglcd2("+");
	odisp2(a.dmg[a.attack]);	
}
void odisp2(int x)
{ 
  switch(x){
    case 0:
      pmsglcd2("0");
      break;
    case 1:
      pmsglcd2("1");
      break;
    case 2:
      pmsglcd2("2");
      break;
    case 3:
      pmsglcd2("3");
      break;  
    case 4:
      pmsglcd2("4");
      break;
    case 5:
      pmsglcd2("5");
      break;
    case 6:
      pmsglcd2("6");
      break;
    case 7:
      pmsglcd2("7");
      break;
    case 8:
      pmsglcd2("8");
      break;
    case 9:
      pmsglcd2("9");
      break;
  }
}
void odisp1(int x)
{ 
  switch(x){
    case 0:
      pmsglcd1("0");
      break;
    case 1:
      pmsglcd1("1");
      break;
    case 2:
      pmsglcd1("2");
      break;
    case 3:
      pmsglcd1("3");
      break;  
    case 4:
      pmsglcd1("4");
      break;
    case 5:
      pmsglcd1("5");
      break;
    case 6:
      pmsglcd1("6");
      break;
    case 7:
      pmsglcd1("7");
      break;
    case 8:
      pmsglcd1("8");
      break;
    case 9:
      pmsglcd1("9");
      break;
  }
}


void hpdisp1(int x)
{
  H = x / 100;
  T = (x - H * 100) / 10;
  O = x - H * 100 - T * 10;
  switch(H){
    case 0:
      pmsglcd1("0");
      break;
    case 1:
      pmsglcd1("1");
      break;
    case 2:
      pmsglcd1("2");
      break;
    case 3:
      pmsglcd1("3");
      break;  
    case 4:
      pmsglcd1("4");
      break;
    case 5:
      pmsglcd1("5");
      break;
    case 6:
      pmsglcd1("6");
      break;
    case 7:
      pmsglcd1("7");
      break;
    case 8:
      pmsglcd1("8");
      break;
    case 9:
      pmsglcd1("9");
      break;
  }
  switch(T){
    case 0:
      pmsglcd1("0");
      break;
    case 1:
      pmsglcd1("1");
      break;
    case 2:
      pmsglcd1("2");
      break;
    case 3:
      pmsglcd1("3");
      break;  
    case 4:
      pmsglcd1("4");
      break;
    case 5:
      pmsglcd1("5");
      break;
    case 6:
      pmsglcd1("6");
      break;
    case 7:
      pmsglcd1("7");
      break;
    case 8:
      pmsglcd1("8");
      break;
    case 9:
      pmsglcd1("9");
      break;
  } 
  switch(O){
    case 0:
      pmsglcd1("0");
      break;
    case 1:
      pmsglcd1("1");
      break;
    case 2:
      pmsglcd1("2");
      break;
    case 3:
      pmsglcd1("3");
      break;  
    case 4:
      pmsglcd1("4");
      break;
    case 5:
      pmsglcd1("5");
      break;
    case 6:
      pmsglcd1("6");
      break;
    case 7:
      pmsglcd1("7");
      break;
    case 8:
      pmsglcd1("8");
      break;
    case 9:
      pmsglcd1("9");
      break;
  }
 
}

void hpdisp2(int x)
{
  H = x / 100;
  T = (x - H * 100) / 10;
  O = x - H * 100 - T * 10;
  switch(H){
    case 0:
      pmsglcd2("0");
      break;
    case 1:
      pmsglcd2("1");
      break;
    case 2:
      pmsglcd2("2");
      break;
    case 3:
      pmsglcd2("3");
      break;  
    case 4:
      pmsglcd2("4");
      break;
    case 5:
      pmsglcd2("5");
      break;
    case 6:
      pmsglcd2("6");
      break;
    case 7:
      pmsglcd2("7");
      break;
    case 8:
      pmsglcd2("8");
      break;
    case 9:
      pmsglcd2("9");
      break;
  }
  switch(T){
    case 0:
      pmsglcd2("0");
      break;
    case 1:
      pmsglcd2("1");
      break;
    case 2:
      pmsglcd2("2");
      break;
    case 3:
      pmsglcd2("3");
      break;  
    case 4:
      pmsglcd2("4");
      break;
    case 5:
      pmsglcd2("5");
      break;
    case 6:
      pmsglcd2("6");
      break;
    case 7:
      pmsglcd2("7");
      break;
    case 8:
      pmsglcd2("8");
      break;
    case 9:
      pmsglcd2("9");
      break;
  } 
  switch(O){
    case 0:
      pmsglcd2("0");
      break;
    case 1:
      pmsglcd2("1");
      break;
    case 2:
      pmsglcd2("2");
      break;
    case 3:
      pmsglcd2("3");
      break;  
    case 4:
      pmsglcd2("4");
      break;
    case 5:
      pmsglcd2("5");
      break;
    case 6:
      pmsglcd2("6");
      break;
    case 7:
      pmsglcd2("7");
      break;
    case 8:
      pmsglcd2("8");
      break;
    case 9:
      pmsglcd2("9");
      break;
  }
}

/*
***********************************************************************
  rdisp1: Displays the two-digit number that is an argument
          on LCD1
***********************************************************************
*/

void rdisp1(int x)
{
  T = x / 10;
  O = x - T * 10;

  switch(T){
    case 0:
      break;
    case 1:
      pmsglcd1("1");
      break;
    case 2:
      pmsglcd1("2");
      break;
    case 3:
      pmsglcd1("3");
      break;  
    case 4:
      pmsglcd1("4");
      break;
    case 5:
      pmsglcd1("5");
      break;
    case 6:
      pmsglcd1("6");
      break;
    case 7:
      pmsglcd1("7");
      break;
    case 8:
      pmsglcd1("8");
      break;
    case 9:
      pmsglcd1("9");
      break;
  } 
  switch(O){
    case 0: 
      pmsglcd1("0");
      break;
    case 1:
      pmsglcd1("1");
      break;
    case 2:
      pmsglcd1("2");
      break;
    case 3:
      pmsglcd1("3");
      break;  
    case 4:
      pmsglcd1("4");
      break;
    case 5:
      pmsglcd1("5");
      break;
    case 6:
      pmsglcd1("6");
      break;
    case 7:
      pmsglcd1("7");
      break;
    case 8:
      pmsglcd1("8");
      break;
    case 9:
      pmsglcd1("9");
      break;
  }
 
}

/*
***********************************************************************
  rdisp2: Displays the two-digit number that is an argument
          on LCD2
***********************************************************************
*/

void rdisp2(int x)
{
  T = x / 10;
  O = x - T * 10;

  switch(T){
    case 0: 
      break;
    case 1:  
      pmsglcd2("1");
      break;
    case 2:
      pmsglcd2("2");
      break;
    case 3:
      pmsglcd2("3");
      break;  
    case 4:
      pmsglcd2("4");
      break;
    case 5:
      pmsglcd2("5");
      break;
    case 6:  
      pmsglcd2("6");
      break;
    case 7:  
      pmsglcd2("7");
      break;
    case 8:
      pmsglcd2("8");
      break;
    case 9:
      pmsglcd2("9");
      break;
  } 
  switch(O){
    case 0:
      pmsglcd2("0");
      break;
    case 1:
      pmsglcd2("1");
      break;
    case 2:
      pmsglcd2("2");
      break;
    case 3:
      pmsglcd2("3");
      break;  
    case 4:
      pmsglcd2("4");
      break;
    case 5:
      pmsglcd2("5");
      break;
    case 6:
      pmsglcd2("6");
      break;
    case 7:
      pmsglcd2("7");
      break;
    case 8:
      pmsglcd2("8");
      break;
    case 9:
      pmsglcd2("9");
      break;
  }
             
}

/*
***********************************************************************
  shiftout1: Transmits the character x to external shift 
            register using the SPI.  It should shift MSB first.  
             
            MISO = PM[4]
            SCK  = PM[5]
***********************************************************************
*/
 
void shiftout1(char x)

{ 
  PTT &= ~SS;              
  while(!(SPISR & 0x20)){}
  SPIDR = x;
  for(i = 0; i < 50; i++){
  }
  // read the SPTEF bit, continue if bit is 1
  // write data to SPI data register
  // wait for 30 cycles for SPI data to shift out 

}

/*
***********************************************************************
  shiftout2: Transmits the character x to external shift 
            register using the SPI.  It should shift MSB first.  
             
            MISO = PM[4]
            SCK  = PM[5]
***********************************************************************
*/
 
void shiftout2(char x)

{ 
  PTT |= SS;               
  while(!(SPISR & 0x20)){}
  SPIDR = x;
  for(i = 0; i < 50; i++){
  }
  // read the SPTEF bit, continue if bit is 1
  // write data to SPI data register
  // wait for 30 cycles for SPI data to shift out 

}

/*
***********************************************************************
  lcdwait: Delay for approx 2 ms
***********************************************************************
*/

void lcdwait()
{          
    for(i = 0; i < 5000; i++){
    }
 
}

/*
*********************************************************************** 
  send_byte1: writes character x to the LCD1
***********************************************************************
*/

void send_byte1(char x)
{           
     shiftout1(x);
     PTT &= ~LCDCLK1;
     PTT |= LCDCLK1;  
     PTT &= ~LCDCLK1; 
     lcdwait();
     // shift out character
     // pulse LCD1 clock line low->high->low
     // wait 2 ms for LCD to process data
}

/*
*********************************************************************** 
  send_byte2: writes character x to the LCD2
***********************************************************************
*/

void send_byte2(char x)
{           
     shiftout2(x); 
     PTT &= ~LCDCLK2;
     PTT |= LCDCLK2;
     PTT &= ~LCDCLK2;
     lcdwait();
     // shift out character
     // pulse LCD2 clock line low->high->low
     // wait 2 ms for LCD to process data
}

/*
***********************************************************************
  send_i1: Sends instruction byte x to LCD1  
***********************************************************************
*/

void send_i1(char x)
{     
        PTT &= ~RS1;
        send_byte1(x);
        // set the register select line low (instruction data)
        // send byte
}

/*
***********************************************************************
  send_i2: Sends instruction byte x to LCD2  
***********************************************************************
*/

void send_i2(char x)
{     
        PTT &= ~RS2;
        send_byte2(x);
        // set the register select line low (instruction data)
        // send byte
}

/*
***********************************************************************
  chgline1: Move LCD1 cursor to position x
  NOTE: Cursor positions are encoded in the LINE1/LINE2 variables
***********************************************************************
*/

void chgline1(char x)
{      
   send_i1(CURMOV);
   send_i1(x);

}
/*
***********************************************************************
  chgline2: Move LCD2 cursor to position x
  NOTE: Cursor positions are encoded in the LINE1/LINE2 variables
***********************************************************************
*/

void chgline2(char x)
{      
   send_i2(CURMOV);
   send_i2(x);

}

/*
***********************************************************************
  print_c1: Print (single) character x on LCD1            
***********************************************************************
*/
 
void print_c1(char x)
{    
   PTT |= RS1;
   send_byte1(x);
            
}

/*
***********************************************************************
  print_c2: Print (single) character x on LCD2            
***********************************************************************
*/
 
void print_c2(char x)
{    
   PTT |= RS2;
   send_byte2(x);

}

/*
***********************************************************************
  pmsglcd1: print character string str[] on LCD1
***********************************************************************
*/

void pmsglcd1(char str[])
{  
  j = 0;
  while(*(str + j)){
    print_c1(*(str + j));
    j++;
  }

}

/*
***********************************************************************
  pmsglcd2: print character string str[] on LCD2
***********************************************************************
*/

void pmsglcd2(char str[])
{  
  j = 0;
  while(*(str + j)){
    print_c2(*(str + j));
    j++;
  }

}

/*
***********************************************************************
  Win: Displays "Players win!" on both LCDs
       Resets player/monster lists
***********************************************************************
*/

void win(void){
  send_i1(LCDCLR);
  chgline1(LINE1);
  send_i2(LCDCLR);
  chgline2(LINE1);
  pmsglcd1("Players win!");
  pmsglcd2("Players win!");
  clearplayers(); 
  PWMDTY3 = PWMPER2 / 2;
  for(k = 0; k < 1200; k++){
    lcdwait();
  }
  PWMPER3 = 0xBF;
  PWMDTY3 = PWMPER3 / 2;
  for(k = 0; k < 1200; k++){
    lcdwait();
  }
  PWMDTY3 = 0;
  while(!(PBGM1 | PBGM2 | PBPC1 | PBPC2));
}

/*
***********************************************************************
  Lose: Displays "Players lose..." on both LCDs
        Resets player/monster lists
***********************************************************************
*/

void lose(void){
  send_i1(LCDCLR);
  chgline1(LINE1);
  send_i2(LCDCLR);
  chgline2(LINE1);
  pmsglcd1("Players lose...");
  pmsglcd2("Players lose...");
  clearplayers();  
  PWMDTY3 = PWMPER2 / 2;
  for(k = 0; k < 1200; k++){
    lcdwait();
  }
  PWMSCLB = 0x60;
  PWMDTY3 = PWMPER3 / 2;
  for(k = 0; k < 1200; k++){
    lcdwait();
  }
  PWMDTY3 = 0;
  PWMSCLB = 0x40;
  while(!(PBGM1 | PBGM2 | PBPC1 | PBPC2));
}

void clearpb(void){
  PBPC1 = 0;
  PBPC2 = 0;
  PBGM1 = 0;
  PBGM2 = 0;
}

/*
***********************************************************************
  clearplayers: Resets player/monster lists
***********************************************************************
*/
void clearplayers(void){
  pnum = 0;
  mnum = 0;
  deadp = 0;
  deadm = 0;
}

/*
***********************************************************************
 Character I/O Library Routines for 9S12C32 
***********************************************************************
 Name:         inchar
 Description:  inputs ASCII character from SCI serial port and returns it
 Example:      char ch1 = inchar();
***********************************************************************
*/

char inchar(void) {
  /* receives character from the terminal channel */
        while (!(SCISR1 & 0x20)); /* wait for input */
    return SCIDRL;
}

/*
***********************************************************************
 Name:         outchar    (use only for DEBUGGING purposes)
 Description:  outputs ASCII character x to SCI serial port
 Example:      outchar('x');
***********************************************************************
*/

void outchar(char x) {
  /* sends a character to the terminal channel */
    while (!(SCISR1 & 0x80));  /* wait for output buffer empty */
    SCIDRL = x;
}
