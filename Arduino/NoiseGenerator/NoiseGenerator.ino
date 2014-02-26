#define FREQ_CPU 16000000
#define PWM_FREQ 0x00FF // 16MHz/333 ~= 48048 -- 14D
                        // 16MHz/362 ~= 44199 -- 16A
#define PWM_MODE 1 // Fast (1) or Phase Correct (0)
#define PWM_QTY 2 // number of pwms, either 1 or 2

#define ITER_GAUSS 1 //Maximum 255 (256 en théorie mais utilisation d'un compteur sur 8 bits) -- Nombre d'itération pour l'approximation de la loi normale (12 par défaut)
#define NB_OLD_SAMPLE 100 //Nombre d'échantillon gardé en mémoire (adapter le type de iOld pour éviter les débordement)

#define N 8 //Valeur de N pour l'algo du bruit rose
#define deuxN 256 //Résultat de 2^N

//Variables pour le générateur pseudo aléatoire
char a,b,c; //unsigned ?
uint8_t x=0;

//Pointeur du bruit à générer
void (*GenBruit)(); //Adresse de la fonction de génération de bruit

//Variables échantillons en cours
float A=1; //Amplification de la sortie (Volume)
char R, L; //échentillon sonore droit et gauche

//Variables tableau historiques
char tabOldSampleR[NB_OLD_SAMPLE]; //Historique des échantillons
char tabOldSampleL[NB_OLD_SAMPLE]; //Historique des échantillons
uint8_t iOld=0; //Index de parcourt des tableaux de l'historique
float aOld=0.1; //Atténuation des valeur de l'historique pour le traitement de retard
uint8_t retardOld=50; //Retard de l'historique -- Ne doit jamais être suppérieur à NB_OLD_SAMPLE

void setup() {
  cli();
  
  GenBruit = bBlancGauss;
  
  // setup PWM
  TCCR1A = (((PWM_QTY - 1) << 5) | 0x80 | (PWM_MODE << 1)); // 
  TCCR1B = ((PWM_MODE << 3) | 0x11); // ck/1
  TIMSK1 = 0x20; // interrupt on capture interrupt
  ICR1H = (PWM_FREQ >> 8);
  ICR1L = (PWM_FREQ & 0xff);
  DDRB |= ((PWM_QTY << 1) | 0x02); // turn on outputs
  
  srand(analogRead(0));
  init_rng(rand(),rand(),rand());
  R=rand();
  L=rand();
  
  sei(); // turn on interrupts - not really necessary with arduino
}

void loop() {
  while(1)
  {/*
    delay(1000);
    cli();
    GenBruit=bBlancGauss;
    sei();
    delay(1000);
    cli();
    GenBruit=bBlancUnif;
    sei();*/
  }
}

ISR(TIMER1_CAPT_vect) {
  //OCR1AH et OCR1BH ne sont pas utiles car la PWM a une résolution de 8 bits
  GenBruit();
  //bOndul();
  OCR1AL = A*R;
  OCR1BL = A*L;
  tabOldSampleR[iOld]=R;
  tabOldSampleL[iOld]=L;
  iOld=(iOld+1)%NB_OLD_SAMPLE;
}

/** Fonctions **/
void ModulationVolume(uint16_t* Freq_mHz, uint16_t* i)
{
//  FREQ_CPU/PWM_FREQ
}

/** Algorithme génération de bruit **/
//Bruit blanc uniforme
void bBlancUnif()
{
  R = randomize();
  L = randomize(); 
}

//Bruit blanc gaussien
void bBlancGauss()
{
  uint8_t i;
  short tempR=0;
  short tempL=0;
  
  for(i=1;i<=ITER_GAUSS;i++)
  {
    tempR += randomize();
    tempL += randomize();
  }
  
  R = (tempR/ITER_GAUSS);
  L = (tempL/ITER_GAUSS);
}

//Bruit rose
void bRoseM1() //Méthode 1 (ne fonctionne certainement pas) -- On execute l'algo pour N = infini
{
  R+=(randomize() % 14);
  L+=(randomize() % 14);
}

 //Partie 1
void bRose() //Méthode 2 (plus lourd) -- On exécute l'algo pour un N défini de manière infini (génération d'une infinité de morceau avec lecture en temps réel)
{
  static bool encours1=false;
  static bool encours2=false;
  static uint8_t oldbit=0xFF;
  static uint8_t K=0;
  
  if(encours2)
  {
    bRose_(&encours2, &K, &oldbit);
  }
  else
  {
    if(!encours1)
    {
      encours1=true;
      K=0;
      oldbit=0xFF;
    }
    R=0;
    L=0;
    bRose_(&encours2, &K, &oldbit);
  }
  K+=1;
  if(K >= deuxN)
  {
    encours1=false;
  }
}

 //Partie 2
void bRose_(bool* encours, uint8_t* K, uint8_t* oldbit)
{
  static uint8_t i=0;
  
  if(!(*encours))
  {
    *encours=true;
    i=0;
  }
  if(((*K)&(0x80>>i))!=((*oldbit)&(0x80>>i)))
  {
    
  }
  
  i+=1;
  if(i>=N)
  {
    *encours=false;
  }
}

//Bruit brownien
void bBrownien()
{
  float tempR, tempL;
  
  tempR=R + (randomize() % 14); //0.1 fois la valeur max de randomize() -- 127*0.1~=13 -- On va donc de -13 à +13
  while(tempR > 127 || tempR < -128)
  {
    tempR=R + (randomize() % 14);
  }
  tempL=L + (randomize() % 14);
  while(tempL > 127 || tempL < -128)
  {
    tempL=L + (randomize() % 14);
  }
  
  R=tempR;
  L=tempL;
}

//Bruit ondulant = retard ?
void bOndul()
{
  short temp;
  
  temp=R+(aOld*tabOldSampleR[iOld-retardOld]);
  if(temp>127)
  {
    R=127;
  }
  else if(temp < -128)
  {
    R=-128;
  }
  else
  {
    R=temp;
  }
    
  temp=L+(aOld*tabOldSampleL[iOld-retardOld]);
  if(temp>127)
  {
    L=127;
  }
  else if(temp < -128)
  {
    L=-128;
  }
  else
  {
    L=temp;
  }
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
void init_rng(uint8_t s1,uint8_t s2,uint8_t s3) //Can also be used to seed the rng with more entropy during use.
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

char randomize()
{
	x++;               //x is incremented every round and is not affected by any other variable
	a = (a^c^x);       //note the mix of addition and XOR
	b = (b+a);         //And the use of very few instructions
	c = (c+(b>>1)^a);  //the right shift is to ensure that high-order bits from b can affect  
	return c;         //low order bits of other variables
}
