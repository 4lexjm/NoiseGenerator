#define PWM_FREQ 0x014D // 16MHz/333 ~= 48048 -- 14D
                        // 16MHz/362 ~= 44199 -- 16A
#define PWM_MODE 1 // Fast (1) or Phase Correct (0)
#define PWM_QTY 2 // number of pwms, either 1 or 2

#define ITER_GAUSS 12 //Nombre d'itération pour l'approximation de la loi normale (12 par défaut)

unsigned char a,b,c;
unsigned char x=0;
float vol=1; //Amplification de la sortie (Volume)
void (*GenBruit)(char* R, char* L, float* A) = bBlancUnif;

void setup() {
  // setup PWM
  TCCR1A = (((PWM_QTY - 1) << 5) | 0x80 | (PWM_MODE << 1)); // 
  TCCR1B = ((PWM_MODE << 3) | 0x11); // ck/1
  TIMSK1 = 0x20; // interrupt on capture interrupt
  ICR1H = (PWM_FREQ >> 8);
  ICR1L = (PWM_FREQ & 0xff);
  DDRB |= ((PWM_QTY << 1) | 0x02); // turn on outputs
  
  srand(analogRead(0));
  init_rng(rand(),rand(),rand());
  
  sei(); // turn on interrupts - not really necessary with arduino
}

void loop() {
  while(1)
  {
    delay(5000);
    GenBruit=bBlancGauss;
    delay(5000);
    GenBruit=bBlancUnif;
  }
}

ISR(TIMER1_CAPT_vect) {
  //OCR1AH et OCR1BH ne sont pas utiles car la PWM a une résolution de 8 bits
  char R, L;
  GenBruit(&R, &L, &vol);
  OCR1AL = R;
  OCR1BL = L;
}

/** Algorithme génération de bruit **/
//Bruit blanc uniforme
void bBlancUnif(char* R, char* L, float* A)
{
  *R = (*A)*randomize();
  *L = (*A)*randomize();
}

//Bruit blanc gaussien
void bBlancGauss(char* R, char* L, float* A)
{
  char i;
  uint16_t tempR=0;
  uint16_t tempL=0;
  
  for(i=0;i<ITER_GAUSS;i++)
  {
    tempR += randomize();
    tempL += randomize();
  }
  *R = (*A)*(tempR/ITER_GAUSS);
  *L = (*A)*(tempL/ITER_GAUSS);
}

/** CORDIC **/ // -- Non implémenté, voir si sin() et cos() de la lib standart du C est assez rapide
/* Bases récupérées sur http://fr.wikipedia.org/wiki/CORDIC
  Calcul de 'cos' et 'sin' d'un angle 'beta' (en radians) par
  l'algorithme CORDIC.
  'n' est le nombre d'itérations (la précision augmente avec lui).
*/
/*
float cosC(float beta, char n)
{
  float K=0.6073; // Valeur de K
  float x = K; // Valeur approchante de cos(beta)
  float x_Nouveau; // Variable temporaire
  float Pow2; // Valeur de la puissance de deux
  
  char i; // declaration de l'indice d'iteration
  for(i = 0; i < nb_iter; i++)
  {
    Pow2 = pow(2,-i);
    // Si beta<0 rotation dans le sens trigo
    if(beta < 0)
    {
      x_Nouveau = x + y*Pow2;
      y -= x*Pow2;
      beta += atan(Pow2);
    }
    // sinon dans l'autre sens
    else
    {
      x_Nouveau = x - y*Pow2;
      y += x*Pow2;
      beta -= atan(Pow2);
    }
    x = x_Nouveau;
  }
}

float sinC(float beta, int n)
{
  float v=0;
}*/


/** Generateur de nombre pseudo aléatoire **/
void init_rng(unsigned char s1,unsigned char s2,unsigned char s3) //Can also be used to seed the rng with more entropy during use.
{
	//XOR new entropy into key state
	a ^=s1;
	b ^=s2;
	c ^=s3;

	x++;
	a = (a^c^x);
	b = (b+a);
	c = (c+(b>>1)^a);
}

unsigned char randomize()
{
	x++;               //x is incremented every round and is not affected by any other variable
	a = (a^c^x);       //note the mix of addition and XOR
	b = (b+a);         //And the use of very few instructions
	c = (c+(b>>1)^a);  //the right shift is to ensure that high-order bits from b can affect  
	return c;         //low order bits of other variables
}

