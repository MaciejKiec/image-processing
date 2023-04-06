#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<math.h>
#include<time.h>

#define W_OK 0                   /* wartosc oznaczajaca brak bledow */
#define B_NIEPOPRAWNAOPCJA -1    /* kody bledow rozpoznawania opcji */
#define B_BRAKNAZWY   -2
#define B_BRAKWARTOSCI  -3
#define B_BRAKPLIKU   -4

typedef struct {
    int typ;
    char *kolor;
    int szarosci;
    int wymx;
    int wymy;
    void *pixel;
} obraz_t;

/* strukura do zapamietywania opcji podanych w wywolaniu programu */
typedef struct {
  FILE *plik_we, *plik_wy;        /* uchwyty do pliku wej. i wyj. */
  int negatyw,progowanie,konturowanie,korekcja_gamma,histogram,wyswietlenie,przetw_kolor;      /* opcje */
  int w_progu;              /* wartosc progu dla opcji progowanie */ 
  float w_gamma;
  char *nazwa_wej, *nazwa_wyj, *rodzaj_kolor; /*nazwa pliku wejściowego i wyjściowego*/
} t_opcje;


/*******************************************************/
/* Funkcja inicjuje strukture wskazywana przez wybor   */
/* PRE:                                                */
/*      poprawnie zainicjowany argument wybor (!=NULL) */
/* POST:                                               */
/*      "wyzerowana" struktura wybor wybranych opcji   */
/*******************************************************/

void wyzeruj_opcje(t_opcje * wybor) {
  wybor->plik_we=NULL;
  wybor->plik_wy=NULL;
  wybor->negatyw=0;
  wybor->konturowanie=0;
  wybor->progowanie=0;
  wybor->wyswietlenie=0;
  wybor->nazwa_wej=0;
  wybor->nazwa_wyj=0;
  wybor->korekcja_gamma=0;
  wybor->histogram=0;
  wybor->w_progu=0;
  wybor->w_gamma=0;
  wybor->rodzaj_kolor=0;
  wybor->przetw_kolor=0;

}

/************************************************************************/
/* Funkcja rozpoznaje opcje wywolania programu zapisane w tablicy argv  */
/* i zapisuje je w strukturze wybor                                     */
/* Skladnia opcji wywolania programu                                    */
/*         program {[-i nazwa] [-o nazwa] [-p liczba] [-n] [-r] [-d] }  */
/* Argumenty funkcji:                                                   */
/*         argc  -  liczba argumentow wywolania wraz z nazwa programu   */
/*         argv  -  tablica argumentow wywolania                        */
/*         wybor -  struktura z informacjami o wywolanych opcjach       */
/* PRE:                                                                 */
/*      brak                                                            */
/* POST:                                                                */
/*      funkcja otwiera odpowiednie pliki, zwraca uchwyty do nich       */
/*      w strukturze wyb�r, do tego ustawia na 1 pola dla opcji, ktore  */
/*	poprawnie wystapily w linii wywolania programu,                 */
/*	pola opcji nie wystepujacych w wywolaniu ustawione sa na 0;     */
/*	zwraca wartosc W_OK (0), gdy wywolanie bylo poprawne            */
/*	lub kod bledu zdefiniowany stalymi B_* (<0)                     */
/* UWAGA:                                                               */
/*      funkcja nie sprawdza istnienia i praw dostepu do plikow         */
/*      w takich przypadkach zwracane uchwyty maja wartosc NULL         */
/************************************************************************/

int przetwarzaj_opcje(int argc, char **argv, t_opcje *wybor) {
  int i, prog;
  float wspol;
  char *kolor_rodzaj = malloc(sizeof(char));

  wyzeruj_opcje(wybor);
  wybor->plik_wy=stdout;        /* na wypadek gdy nie podano opcji "-o" */

  for (i=1; i<argc; i++) {
    if (argv[i][0] != '-')  /* blad: to nie jest opcja - brak znaku "-" */
      return B_NIEPOPRAWNAOPCJA; 
    switch (argv[i][1]) {
    case 'i': {                 /* opcja z nazwa pliku wejsciowego */
      if (++i<argc) {   /* wczytujemy kolejny argument jako nazwe pliku */
  wybor->nazwa_wej=argv[i];
	if (strcmp(wybor->nazwa_wej,"-")==0) /* gdy nazwa jest "-"        */
	  wybor->plik_we=stdin;            /* ustwiamy wejscie na stdin */
	else                               /* otwieramy wskazany plik   */
	  wybor->plik_we=fopen(wybor->nazwa_wej,"r");
      } else 
	return B_BRAKNAZWY;                   /* blad: brak nazwy pliku */
      break;
    }
    case 'o': {                 /* opcja z nazwa pliku wyjsciowego */
      if (++i<argc) {   /* wczytujemy kolejny argument jako nazwe pliku */
  wybor->nazwa_wyj=argv[i];
	if (strcmp(wybor->nazwa_wyj,"-")==0)/* gdy nazwa jest "-"         */
	  wybor->plik_wy=stdout;          /* ustwiamy wyjscie na stdout */
	else                              /* otwieramy wskazany plik    */
	  wybor->plik_wy=fopen(wybor->nazwa_wyj,"w");
      } else 
	return B_BRAKNAZWY;                   /* blad: brak nazwy pliku */
      break;
    }
    case 'p': {
      if (++i<argc) { /* wczytujemy kolejny argument jako wartosc progu */
	if (sscanf(argv[i],"%d",&prog)==1) {
      if(prog<0 || prog>100) return B_BRAKWARTOSCI; /*sprawdzamy, czy próg zawiera się między 0 a 100 */
	  wybor->progowanie=1;
	  wybor->w_progu=prog;
	} else
	  return B_BRAKWARTOSCI;     /* blad: niepoprawna wartosc progu */
      } else 
	return B_BRAKWARTOSCI;             /* blad: brak wartosci progu */
      break;
    }
    case 'm': {
      if (++i<argc) { /* wczytujemy kolejny argument jako wartosc progu */
	if (sscanf(argv[i],"%s",kolor_rodzaj)==1) {
    char temp[512];
    if(wybor->plik_we==NULL){
      fprintf(stderr,"Nie udało się odczytać podanego pliku!\n");
      exit(0);
    }
    fgets(temp,512,wybor->plik_we);
      if(temp[1]!='3'){
        fprintf(stderr,"Blad: Wczytany plik nie jest obrazem ppm\n");
        exit(0);
      }
      if(strcmp(kolor_rodzaj,"r")!=0 && strcmp(kolor_rodzaj,"g")!=0 && strcmp(kolor_rodzaj,"b") && strcmp(kolor_rodzaj,"s")!=0){
        return B_BRAKWARTOSCI;
        exit(0);
      }
	  wybor->przetw_kolor=1;
    wybor->rodzaj_kolor = kolor_rodzaj;
	} else
	  return B_BRAKWARTOSCI;     /* blad: niepoprawna wartosc progu */
      } else 
	return B_BRAKWARTOSCI;             /* blad: brak wartosci progu */
      break;
    }
    case 'n': {                 /* mamy wykonac negatyw */
      wybor->negatyw=1;
      break;
    }
    case 'k': {                 /* mamy wykonac konturowanie */
      wybor->konturowanie=1;
      break;
    }
    case 'h': {                 /* mamy wykonac rozciaganie histogramu */
      wybor->histogram=1;
      break;
    }
    case 'g': {
      if (++i<argc) { /* wczytujemy kolejny argument jako wartosc wspolczynnika gamma */
	if (sscanf(argv[i],"%e",&wspol)==1) {
      if(wspol<=0) return B_BRAKWARTOSCI;
	  wybor->korekcja_gamma=1;
	  wybor->w_gamma=wspol;
	} else
	  return B_BRAKWARTOSCI;     /* blad: niepoprawna wartosc progu */
      } else 
	return B_BRAKWARTOSCI;             /* blad: brak wartosci progu */
      break;
    }
    case 'd': {                 /* mamy wyswietlic obraz */
      wybor->wyswietlenie=1;
      break;
    }
    default:                    /* nierozpoznana opcja */
      return B_NIEPOPRAWNAOPCJA;
    } /*koniec switch */
  } /* koniec for */

  if (wybor->plik_we!=NULL)     /* ok: wej. strumien danych zainicjowany */
    return W_OK;
  else 
    return B_BRAKPLIKU;         /* blad:  nie otwarto pliku wejsciowego  */
}