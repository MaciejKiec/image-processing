#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<math.h>
#include<time.h>
#define MAX 512  

typedef struct {
    int typ;
    char *kolor;
    int nasycenie;
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

int main(int argc, char ** argv) {
  obraz_t obraz_dyn;
  t_opcje opcje;
  int zmiany[6]; // zmienna do obsługi zapisu zmian w pliku
  float wspol[2]; //zmienna do obsługi zapisu zmian w pliku
  float *w_wspol = wspol;
  int kon_do_szarosci = 0;
  char komentarz [MAX]; //zmienna przechowująca komentarz użytkownika
  komentarz[0] = 0;

  switch(przetwarzaj_opcje(argc,argv,&opcje)){
    case -4:
    printf("Nie udało się odczytać podanego pliku!\n");
    remove(opcje.nazwa_wyj);
    exit(0);
    break;
    case -3:
    printf("Wprowadzono nieprawidłowe wartości!\n");
    remove(opcje.nazwa_wyj);
    exit(0);
    break;
    case -2:
    printf("Nie podano nazwy pliku!\n");
    remove(opcje.nazwa_wyj);
    exit(0);
    break;
    case -1:
    printf("Nie rozpoznano opcji!\n");
    remove(opcje.nazwa_wyj);
    exit(0);
    break;
  }




  if(opcje.plik_we!=stdin){
    czytaj(opcje.plik_we,&obraz_dyn);
  }else{
    czytaj(stdin,&obraz_dyn);
  }
  if(opcje.przetw_kolor==1 && obraz_dyn.typ==3){
    obraz_dyn.kolor = opcje.rodzaj_kolor;
  }else if(opcje.przetw_kolor==0 && obraz_dyn.typ==3){
    printf("Nie podano koloru do przetwarzania!\n");
    exit(0);
  }
   if(opcje.negatyw==1){
    negatyw(&obraz_dyn,zmiany);
  }
   if(opcje.konturowanie==1){
    konturowanie(&obraz_dyn,zmiany);
  }
   if(opcje.progowanie==1){
    progowanie(&obraz_dyn,opcje.w_progu,zmiany,w_wspol);
  }
   if(opcje.korekcja_gamma==1){
    korekcja_gamma(&obraz_dyn,opcje.w_gamma,zmiany,w_wspol);
  }
   if(opcje.histogram==1){
   histogram(&obraz_dyn,zmiany);
  }
  if(opcje.przetw_kolor==1 && strcmp(opcje.rodzaj_kolor,"s")==0){
    kon_do_szarosci = konwersja(&obraz_dyn,zmiany);
  }
  
  if(opcje.wyswietlenie==1){
    wyswietl(&obraz_dyn,&opcje,zmiany,komentarz,wspol,kon_do_szarosci);
  }
  if(opcje.plik_wy!=stdout && opcje.nazwa_wyj!=NULL){
    zapisz(opcje.plik_wy,&obraz_dyn,opcje.nazwa_wyj,zmiany,komentarz,wspol,kon_do_szarosci);
    fclose(opcje.plik_wy);
    printf("Zapisano obraz pod nazwą: %s\n", opcje.nazwa_wyj);
  }else if(opcje.plik_wy==stdout && opcje.nazwa_wyj!=NULL && strcmp(opcje.nazwa_wyj,"-")==0){
    zapisz(stdout,&obraz_dyn,opcje.nazwa_wyj,zmiany,komentarz,wspol,kon_do_szarosci);
  }

  return 0;
}
