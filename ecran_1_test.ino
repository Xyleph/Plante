#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <TimeLib.h>

//les donnes de chaque plante se trouve dans une structure plante
struct plante
{
  //ces valeurs n'ont pas besoin d'etre modifiees manuellement
  time_t dernier_arrosage; //time code de la derniere fois que cette plante a ete arrosee
  int lecture_capteur;     //derniere lecture du capteur d'humidite
  //ces valeurs doivent etre modifiees manuellement dans la partie setup()
  String nom;
  String piece;
  int limite_humidite;     //treshold au dessus duquel la plante sera arrosee (0-1023, sujet a ajustement)
  int analog_capteur;      //pin analog du capteur de cette plante
  int pompe;               //pin digital de la base du transistor de cette plante
};

#define NB_PLANTE  2
#define PIN_DROITE 22        //pin digital du bouton de controle droit 
#define PIN_CENTRE 24        //pin digital du bouton de controle centre
#define PIN_GAUCHE 26        //pin digital du bouton de controle gauche
#define PIN_POWER_CAPTEUR 30 //pin digital de l'alimentation des capteurs

plante liste_plante[NB_PLANTE];

time_t derniere_lecture;           //time code de la derniere lecture des capteurs
LiquidCrystal_I2C lcd(0x27,20,4);
int bouton[3];                     //contient l'etat des boutons pour cette lecture
int i;
int menu_vu;
int menu_actif;
int tempslecture;                  //temps entre les lectures en minute. Ajustable dans le menu 1
int tempsarrosage;                 //temps d'arrosage en seconde. Ajustable dans le menu 2
int plante_active;                 //index de la plante a afficher dans le menu 3
bool changement;                   //indique que l'ecran doit etre rafaichi
/*********************************************************/
void setup()
{
  derniere_lecture = now();
  pinMode(PIN_DROITE , INPUT);
  pinMode(PIN_CENTRE , INPUT);
  pinMode(PIN_GAUCHE , INPUT);
  lcd.init();
  lcd.backlight();
  changement = false;
  plante_active = 0;
  menu_vu = 1;
  menu_actif = 0;
  afficher_menu(menu_actif,menu_vu);
  //initialisation des variables des plantes
  for(i = 0 ; i < NB_PLANTE ; i++)
  {
    liste_plante[i].dernier_arrosage = now();
    liste_plante[i].lecture_capteur = 5;
  }
  //les variables nom , piece , limite_humidite , analog_capteur et pompe doivent etre entrees pour chaque plante
  liste_plante[0].nom = "Plante1";
  liste_plante[0].piece = "Ta mere";
  liste_plante[0].limite_humidite = 500;
  liste_plante[0].analog_capteur = 4;
  liste_plante[0].pompe = 32;
  liste_plante[1].nom = "Plante2";
  liste_plante[1].piece = "Sa mere";
  liste_plante[1].limite_humidite = 600;
  liste_plante[1].analog_capteur = 5;
  liste_plante[1].pompe = 34;
}
/*********************************************************/
void loop() 
{
  //Verification du delais depuis la derniere lecture
  if((total_minute()-total_minute(derniere_lecture)) > tempslecture)
  {
    verif_capteur();
    arroser();
  }
  //la soustraction sera negative apres un mois complet, la valeur de lecture est donc reinitialisee
  if((total_minute()-total_minute(derniere_lecture)) < 0)
  {
    derniere_lecture = now();
  }
  
  bouton[0] = digitalRead(PIN_DROITE);
  bouton[1] = digitalRead(PIN_CENTRE);
  bouton[2] = digitalRead(PIN_GAUCHE);

  lire_bouton();
  
  if(changement)
  {
    afficher_menu(menu_actif , menu_vu);
    changement = false;
  }

  delay(100);
}

void verif_capteur()
{
  //Activation des capteurs
  for(i = 0 ; i < NB_PLANTE ; i++)
  {
    digitalWrite(PIN_POWER_CAPTEUR , HIGH);
  }
  delay(20);
  //Lecture des valeurs d'humidite
  for(i = 0 ; i < NB_PLANTE ; i++)
  {
    liste_plante[i].lecture_capteur = analogRead(liste_plante[i].analog_capteur);
  }
  delay(20);
  //Desactivation des capteurs
  for(i = 0 ; i < NB_PLANTE ; i++)
  {
    digitalWrite(PIN_POWER_CAPTEUR , LOW);
  }
}

void arroser()
{
  for(i = 0 ; i < NB_PLANTE ; i++)
  {
    if(liste_plante[i].lecture_capteur > liste_plante[i].limite_humidite)
    {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Arrosage de :");
      lcd.setCursor(0,1);
      lcd.print(liste_plante[i].nom);
      lcd.setCursor(0,2);
      lcd.print("en cours");
      digitalWrite(liste_plante[i].pompe , HIGH);
      delay(tempsarrosage * 1000);
      digitalWrite(liste_plante[i].pompe , LOW);
      liste_plante[i].dernier_arrosage = now();
      changement = true;
    }
  }
}

void afficher_menu(int menuactif , int menuvu)
{
  if (menuactif == 0)
  {
    if (menuvu == 1)
    {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Modifier l'interval");
      lcd.setCursor(0,1);
      lcd.print("de lecture des");
      lcd.setCursor(0,2);
      lcd.print("capteurs d'humidite");
    }
    if (menuvu == 2)
    {
      lcd.clear();
      lcd.setCursor(0,1);
      lcd.print("Modifier la duree");
      lcd.setCursor(0,2);
      lcd.print("d'arrosage");
    }
    if (menuvu == 3)
    {
      lcd.clear();
      lcd.setCursor(0,1);
      lcd.print("Voir l'etat");
      lcd.setCursor(0,2);
      lcd.print("des plantes");
    }
  }
  else if(menuactif == 1)
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Interval actuel");
    lcd.setCursor(0,1);
    lcd.print("entre les lectures");
    lcd.setCursor(5,3);
    lcd.print(tempslecture);
    lcd.print(" minutes");
  }
  else if(menuactif == 2)
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Duree actuel");
    lcd.setCursor(0,1);
    lcd.print("de chaque arrosage");
    lcd.setCursor(5,3);
    lcd.print(tempsarrosage);
    lcd.print(" secondes");
  }
  else if(menuactif == 3)
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Nom : ");
    lcd.print(liste_plante[plante_active].nom);
    lcd.setCursor(0,1);
    lcd.print("Piece : ");
    lcd.print(liste_plante[plante_active].piece);
    lcd.setCursor(0,2);
    lcd.print("Derniere arrosage");
    lcd.setCursor(0,3);
    lcd.print("il y a ");
    lcd.print(day(liste_plante[plante_active].dernier_arrosage));
    lcd.print("j et ");
    lcd.print(hour(liste_plante[plante_active].dernier_arrosage));
    lcd.print("h");
  }
}

void lire_bouton()
{
  //ces if servent a verifier que seulement un bouton a ete appuyer durant la lecture
  if((bouton[0]==1) and (bouton[1]==0) and (bouton[2]==0))
  {
    if(menu_actif == 0)
    {
      if(menu_vu == 3)
      {
        menu_vu = 1;
      }
      else
      {
        menu_vu++;
      }
    }
    else if(menu_actif == 1)
    {
      tempslecture += 5;
    }
    else if(menu_actif == 2)
    {
      tempsarrosage += 1;
    }
    else if(menu_actif == 3)
    {
      if(plante_active == NB_PLANTE-1)
      {
        plante_active = 0;
      }
      else
      {
        plante_active++;
      }
    }
    changement = true;
  }
  else if((bouton[0]==0) and (bouton[1]==1) and (bouton[2]==0))
  {
    if(menu_actif == 0)
    {
      menu_actif = menu_vu;
    }
    else
    {
      menu_actif = 0;
    }
    changement = true;
  }
  else if((bouton[0]==0) and (bouton[1]==0) and (bouton[2]==1))
  {
    if(menu_actif == 0)
    {
      if(menu_vu == 1)
      {
        menu_vu = 3;
      }
      else
      {
        menu_vu--;
      }
    }
    else if(menu_actif == 1)
    {
      if(tempslecture == 0)
      {
      }
      else
      {
        tempslecture -= 5;
      }
    }
    else if(menu_actif == 2)
    {
      if(tempsarrosage == 0)
      {
      }
      else
      {
        tempsarrosage -= 1;
      }
    }
    else if(menu_actif == 3)
    {
      if(plante_active == 0)
      {
        plante_active = NB_PLANTE - 1;
      }
      else
      {
        plante_active--;
      }
    }
    changement = true;
  }
  else
  {
  }
}

int total_minute()
{
  return (minute(now()) + (60 * hour(now())) + (1440 * day(now())));
}

int total_minute(time_t t)
{
  return (minute(t) + (60 * hour(t)) + (1440 * day(t)));
}
/************************************************************/
