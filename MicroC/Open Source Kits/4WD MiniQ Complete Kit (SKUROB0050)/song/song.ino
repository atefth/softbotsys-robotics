// Note timing value frequency hz
#define DO_L E2 // 262
#define DOA_L E4 // 277
#define RE_L E5 // 294
#define REA_L E7 // 311
#define MI_L E8 // 330
#define FA_L EA // 349
#define FAA_L EB // 370
#define SO_L EC // 392
#define SOA_L ED // 415
#define LA_L EE // 440
#define LAA_L EF // 466
#define TI_L F0 // 494
#define DO F1 // 523
#define DOA F2 // 554
#define RE F3 // 587
#define REA F3 // 622
#define MI F4 // 659
#define FA F5 // 698
#define FAA F5 // 740
#define SO F6 // 784
#define SOA F7 // 831
#define LA F7 // 880
#define LAA F8 // 932
#define TI F8 // 988
#define DO_H F9 // 1046
#define DOA_H F9 // 1109
#define RE_H F9 // 1175
#define REA_H FA // 1245
#define MI_H FA // 1318
#define FA_H FA // 1397
#define FAA_H FB // 1480
#define SO_H FB // 1568
#define SOA_H FB // 1661
#define LA_H FC // 1760
#define LAA_H FC // 1865
#define TI_H FC // 1976
#define ZERO 0 // pause

#define BUZZER 11 // define the buzzer pin

int initial_value; // sent when the timer initial value of each note
char time; // time for each note played
char ptr = 0x00; // point to an array of music
char flag = 0;
int music [] = {
  0XF1, 2, 0XF3, 2, 0XF4, 2, 0XF1, 1, 0, 1, // ??two tigers
  0XF1, 2, 0XF3, 2, 0XF4, 2, 0XF1, 1, 0, 1,
  0XF4, 2, 0XF5, 2, 0XF6, 2, 0, 2,
  0XF4, 2, 0XF5, 2, 0XF6, 2, 0, 2,
  0XF6, 1, 0XF7, 1, 0XF6, 1, 0XF5, 1, 0XF4, 2, 0XF1, 2,
  0XF6, 1, 0XF7, 1, 0XF6, 1, 0XF5, 1, 0XF4, 2, 0XF1, 2,
  0XF3, 2, 0XEC, 2, 0XF1, 2, 0, 2,
  0XF3, 2, 0XEC, 2, 0XF1, 2, 0, 2, 0xff};

void timer2_init (void) // initialize timer 2
{
  TCCR2A = 0X00;
  TCCR2B = 0X07; // divide clock source 1024
  TCNT2 = initial_value;
  TIMSK2 = 0X01; // enable interrupt
}
ISR (TIMER2_OVF_vect) // Timer 2 interrupt
{
  TCNT2 = initial_value; // initial value for the timer
  flag = ~ flag;
  if (flag)
     digitalWrite (BUZZER, HIGH); // set high, the buzzer
  else
    digitalWrite (BUZZER, LOW); // set low, the buzzer did not ring
}
void play_music (void) // Play Music
{
  if (music [ptr] != 0xFF & & music [ptr] != 0x00) // determine whether it is normal notes
  {
    TCCR2B = 0X07; // timer to work
    initial_value = music [ptr]; // set the timer to take the initial value
    time = music [ptr + 1]; // get phonation time
    delay (time * 200); // delay
    ptr += 2; // point to the next note
  }
  else if (music [ptr] == 0x00) // determine whether it is a full stop
  {
    time = music [ptr + 1]; // get phonation time
    delay (time * 200); // delay
    ptr += 2; // point to the next note
  }
  else // is the terminator
  {
    TCCR2B = 0X00; // timer stopped working
    digitalWrite (BUZZER, LOW); // set low, the buzzer did not ring
    delay (time * 200); // delay
    ptr = 0; // clear, easy to start again
  }
}
void setup ()
{
   pinMode (BUZZER, OUTPUT); // set output pin to connect the buzzer mode
   timer2_init ();// timer initialization
   sei ();// open the global interrupt
}
void loop (void)
{
  while (1)
  {
    play_music ();// music
  }

}
