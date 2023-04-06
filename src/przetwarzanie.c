#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<math.h>
#include<time.h>
#define DL_LINII 1024      /* Dlugosc buforow pomocniczych */

typedef struct {
    int typ;
    char *kolor;
    int nasycenie;
    int wymx;
    int wymy;
    void *pixel;
} obraz_t;

typedef struct {
  FILE *plik_we, *plik_wy;        /* uchwyty do pliku wej. i wyj. */
  int negatyw,progowanie,konturowanie,korekcja_gamma,histogram,wyswietlenie,przetw_kolor;      /* opcje */
  int w_progu;              /* wartosc progu dla opcji progowanie */ 
  float w_gamma;
  char *nazwa_wej, *nazwa_wyj, *rodzaj_kolor; /*nazwa pliku wejściowego i wyjściowego*/
} t_opcje;

/************************************************************************************
 * Funkcja wczytuje obraz PGM z pliku do tablicy       	       	       	       	    *
 *										    *
 * \param[in] plik_we uchwyt do pliku z obrazem w formacie PGM			    *
 * \param[out] piksele tablica, do ktorej zostanie zapisany obraz		    *
 * \param[out] wymx szerokosc obrazka						    *
 * \param[out] wymy wysokosc obrazka						    *
 * \param[out] nasycenie liczba odcieni nasycenie					    *
 * \return liczba wczytanych pikseli						    *
 ************************************************************************************/

int czytaj(FILE *plik_we, obraz_t *obraz_dyn) {
  char buf[DL_LINII];      /* bufor pomocniczy do czytania naglowka i komentarzy */
  int znak;                /* zmienna pomocnicza do czytania komentarzy */
  int koniec=0;            /* czy napotkano koniec danych w pliku */
  int i,j;

  /*Sprawdzenie czy podano prawid�owy uchwyt pliku */
  if (plik_we==NULL) {
    fprintf(stderr,"Blad: Nie podano uchwytu do pliku\n");
    return(0);
  }

  /* Sprawdzenie "numeru magicznego" - powinien by� P2 */
  if (fgets(buf,DL_LINII,plik_we)==NULL)   /* Wczytanie pierwszej linii pliku do bufora */
    koniec=1;                              /* Nie udalo sie? Koniec danych! */

  if ( (buf[0]=='P') && (buf[1]=='2') && koniec!=1) {  /* Czy jest magiczne "P2"? */
    obraz_dyn->typ=1;
  }else if( (buf[0]='P') && (buf[1]='3') && koniec!=1){
    obraz_dyn->typ=3;
  }else{
    fprintf(stderr,"Blad: Nie rozpoznano pliku!\n");
    return(0);
  }

  /* Pominiecie komentarzy */
  do {
    if ((znak=fgetc(plik_we))=='#') {         /* Czy linia rozpoczyna sie od znaku '#'? */
      if (fgets(buf,DL_LINII,plik_we)==NULL)  /* Przeczytaj ja do bufora                */
	koniec=1;                   /* Zapamietaj ewentualny koniec danych */
    }  else {
      ungetc(znak,plik_we);                   /* Gdy przeczytany znak z poczatku linii */
    }                                         /* nie jest '#' zwroc go                 */
  } while (znak=='#' && !koniec);   /* Powtarzaj dopoki sa linie komentarza */
                                    /* i nie nastapil koniec danych         */

  /* Pobranie wymiarow obrazu i liczby odcieni szarosci */
  if (fscanf(plik_we,"%d %d %d",&(obraz_dyn->wymx),&(obraz_dyn->wymy),&obraz_dyn->nasycenie)!=3) {
    fprintf(stderr,"Blad: Brak wymiarow obrazu lub liczby stopni szarosci\n");
    return(0);
  }
  obraz_dyn->pixel = malloc(obraz_dyn->wymx*obraz_dyn->wymy*sizeof(int)*obraz_dyn->typ);
  int (*piksele)[obraz_dyn->wymx*obraz_dyn->typ];
  piksele = (int(*)[obraz_dyn->wymx*obraz_dyn->typ]) obraz_dyn->pixel;
  /* Pobranie obrazu i zapisanie w tablicy piksele*/
  for (i=0;i<obraz_dyn->wymy;i++) {
    for (j=0;j<obraz_dyn->wymx*obraz_dyn->typ;j++) {
      if (fscanf(plik_we,"%d",&(piksele[i][j]))!=1) {
	fprintf(stderr,"Blad: Niewlasciwe wymiary obrazu\n");
	return(0);
      }
    }
  }
  return 1;   /* Czytanie zakonczone sukcesem    */
}                       

/*funckja zapisująca*/
int zapisz(FILE *plik_wy, obraz_t *obraz_dyn, char n_nazwa[], int zmiany[5], char komentarz[], float wspol[2],int kon_do_szarosci){
  int add;
  int (*piksele)[obraz_dyn->wymx*obraz_dyn->typ];
  piksele = (int(*)[obraz_dyn->wymx*obraz_dyn->typ]) obraz_dyn->pixel;
  if(obraz_dyn->typ==1){
    fprintf(plik_wy,"P2\n");
  }else if(obraz_dyn->typ==3 && kon_do_szarosci==1){
    fprintf(plik_wy,"P2\n");
  }else if(obraz_dyn->typ==3){
    fprintf(plik_wy,"P3\n");
  }
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  //inrofmacja o czasie zapisu
  fprintf(plik_wy,"#%d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year+1900, tm.tm_mon +1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
  //informacje o wykorzystanych filtrach
  if (zmiany[1] == 1) fprintf(plik_wy,"#Zastosowany został filtr: negatyw\n");
  if (zmiany[2] == 1) fprintf(plik_wy,"#Zastosowany został filtr: progowanie o wartości: %d %%\n", (int) wspol[0]);
  if (zmiany[3] == 1) fprintf(plik_wy,"#Zastosowany został filtr: korekcja gamma o współczynniku: %f\n", wspol[1]);
  if (zmiany[4] == 1) fprintf(plik_wy,"#Zastosowany został filtr: konturowanie\n");
  if (zmiany[5] == 1) fprintf(plik_wy,"#Zastosowany został filtr: rozciąganie histogramu\n");
  if (strlen(komentarz) != 0) fprintf(plik_wy,"#%s\n", komentarz); //dodawanie komentarza, o ile takowy był podany
  fprintf(plik_wy,"%d %d \n", obraz_dyn->wymx,obraz_dyn->wymy); 
  fprintf(plik_wy,"%d\n", obraz_dyn->nasycenie);
  
  add = (kon_do_szarosci ? 3:1);

  for (int y = 0; y<obraz_dyn->wymy; y++){
    for(int x = 0; x<obraz_dyn->wymx*obraz_dyn->typ; x+=add){
      fprintf(plik_wy,"%d ",piksele[y][x]);
    }
    fprintf(plik_wy," \n");
  }
  free(obraz_dyn->pixel);
  return 1;
}

/* Wyswietlenie obrazu o zadanej nazwie za pomoca programu "display"   */
void wyswietl(obraz_t *obraz_dyn, t_opcje *opcje, int zmiany[5],char komentarz[], float wspol[2], int kon_do_szarosci) {
  char polecenie[DL_LINII];      /* bufor pomocniczy do zestawienia polecenia */

  FILE *temp;
  temp = fopen("temp","w");
  zapisz(temp,obraz_dyn,opcje->nazwa_wyj, zmiany, komentarz, wspol, kon_do_szarosci);
  fclose(temp);

  system("display temp &");
  sleep(1);
  system("rm temp"); 
}

int negatyw(obraz_t *obraz_dyn, int zmiany[6]){
  
  int (*piksele)[obraz_dyn->wymx*obraz_dyn->typ];
  piksele = (int(*)[obraz_dyn->wymx*obraz_dyn->typ]) obraz_dyn->pixel;
  int s,shift,add;
  if(obraz_dyn->typ==3){
    if(strcmp(obraz_dyn->kolor,"r")==0){
      s=0;
      shift=0;
      add=3;
    }else if(strcmp(obraz_dyn->kolor,"g")==0){
      s=1;
      shift=1;
      add=3;
    }else if(strcmp(obraz_dyn->kolor,"b")==0){
      s=2;
      shift=0;
      add=3;
    }
  }else{
    s=0;
    shift=0;
    add=1;
  }

    for (int y = 0; y<obraz_dyn->wymy; y++){
      for(int x=s; x<obraz_dyn->wymx*obraz_dyn->typ-shift; x+=add){
        piksele[y][x] = obraz_dyn->nasycenie - piksele[y][x];
      }
    }
  return zmiany[1] = 1;
}

int progowanie(obraz_t *obraz_dyn,int prog, int zmiany[6],float *w_wspol){
   int (*piksele)[obraz_dyn->wymx*obraz_dyn->typ];
   piksele = (int(*)[obraz_dyn->wymx*obraz_dyn->typ]) obraz_dyn->pixel;
  int s,shift,add;
  if(obraz_dyn->typ==3){
    if(strcmp(obraz_dyn->kolor,"r")==0){
      s=0;
      shift=0;
      add=3;
    }else if(strcmp(obraz_dyn->kolor,"g")==0){
      s=1;
      shift=1;
      add=3;
    }else if(strcmp(obraz_dyn->kolor,"b")==0){
      s=2;
      shift=0;
      add=3;
    }
  }else{
    s=0;
    shift=0;
    add=1;
  }

int wprog = obraz_dyn->nasycenie*prog/100;

    for (int y = 0; y<obraz_dyn->wymy; y++){
      for(int x=s; x<obraz_dyn->wymx*obraz_dyn->typ-shift; x+=add){
         if (piksele[y][x]<=wprog){
            piksele[y][x] = 0;
         }else{
           piksele[y][x] = obraz_dyn->nasycenie;
         }
      }
    }
    
  *w_wspol = prog;
  return zmiany[2] = 1;
}

int korekcja_gamma(obraz_t *obraz_dyn, float wspolczynnik, int zmiany[6], float *w_wspol){
   int (*piksele)[obraz_dyn->wymx*obraz_dyn->typ];
   piksele = (int(*)[obraz_dyn->wymx*obraz_dyn->typ]) obraz_dyn->pixel;
   int s,shift,add;
  if(obraz_dyn->typ==3){
    if(strcmp(obraz_dyn->kolor,"r")==0){
      s=0;
      shift=0;
      add=3;
    }else if(strcmp(obraz_dyn->kolor,"g")==0){
      s=1;
      shift=1;
      add=3;
    }else if(strcmp(obraz_dyn->kolor,"b")==0){
      s=2;
      shift=0;
      add=3;
    }
  }else{
    s=0;
    shift=0;
    add=1;
  }
    for (int y = 0; y<obraz_dyn->wymy; y++){
      for(int x=s; x<obraz_dyn->wymx*obraz_dyn->typ-shift; x+=add){
        piksele[y][x] = pow(piksele[y][x]/(double)obraz_dyn->nasycenie,1/wspolczynnik)*(double)obraz_dyn->nasycenie;
      }
    }
    *(w_wspol+1) = wspolczynnik; 

  return zmiany[3] = 1;
}

int konturowanie(obraz_t *obraz_dyn, int zmiany[6]){
   int (*piksele)[obraz_dyn->wymx*obraz_dyn->typ];
   piksele = (int(*)[obraz_dyn->wymx*obraz_dyn->typ]) obraz_dyn->pixel;
   int s,shift,add;
  if(obraz_dyn->typ==3){
    if(strcmp(obraz_dyn->kolor,"r")==0){
      s=0;
      shift=0;
      add=3;
    }else if(strcmp(obraz_dyn->kolor,"g")==0){
      s=1;
      shift=1;
      add=3;
    }else if(strcmp(obraz_dyn->kolor,"b")==0){
      s=2;
      shift=0;
      add=3;
    }
  }else{
    s=0;
    shift=0;
    add=1;
  }

for (int y = 0; y<obraz_dyn->wymy; y++){
      for(int x=s; x<obraz_dyn->wymx*obraz_dyn->typ-shift; x+=add){
        if(x+1 == obraz_dyn->wymx*obraz_dyn->typ){
          if(y+1 == obraz_dyn->wymy){
            piksele[y][x]= 0;
          }else{
            piksele[y][x] = abs(piksele[y+1][x]-piksele[y][x]);
          }
        }else{
          if(y+1 ==obraz_dyn->wymy){
          piksele[y][x] = abs(piksele[y][x+1]-piksele[y][x]); 
          }else{
          piksele[y][x] = abs(piksele[y+1][x]-piksele[y][x]) + abs(piksele[y][x+1]-piksele[y][x]);
          }
        }
      }
    } 

  return zmiany[4] = 1;
}
int histogram(obraz_t *obraz_dyn, int zmiany[6]){
   int (*piksele)[obraz_dyn->wymx*obraz_dyn->typ];
   piksele = (int(*)[obraz_dyn->wymx*obraz_dyn->typ]) obraz_dyn->pixel;

  int true_min = 255;
  int true_max = 0; 

   int s,shift,add;
  if(obraz_dyn->typ==3){
    if(strcmp(obraz_dyn->kolor,"r")==0){
      s=0;
      shift=0;
      add=3;
    }else if(strcmp(obraz_dyn->kolor,"g")==0){
      s=1;
      shift=1;
      add=3;
    }else if(strcmp(obraz_dyn->kolor,"b")==0){
      s=2;
      shift=0;
      add=3;
    }
  }else{
    s=0;
    shift=0;
    add=1;
  }

  for(int y = 0; y<obraz_dyn->wymy; y++){
    for(int x=s; x<obraz_dyn->wymx*obraz_dyn->typ-shift; x+=add){
      if((true_min > piksele[y][x]) && piksele[y][x]!=0){
        true_min = piksele[y][x];
      }
      if((true_max < piksele[y][x]) && piksele[y][x]!=255){
        true_max = piksele[y][x];
    }else if(piksele[y][x] == 255){
      true_max = 255;
    }
  }
  }

  for(int y = 0;y<obraz_dyn->wymy;y++){
    for(int x=s; x<obraz_dyn->wymx*obraz_dyn->typ-shift; x+=add){
      if(piksele[y][x] !=0) piksele[y][x] = (piksele[y][x] - true_min)*(obraz_dyn->nasycenie)/(true_max-true_min);
    }
  }


  return zmiany[5] = 1;
}

int konwersja(obraz_t *obraz_dyn,int zmiany[6]){
    int (*piksele)[obraz_dyn->wymx*obraz_dyn->typ];
    piksele = (int(*)[obraz_dyn->wymx*obraz_dyn->typ]) obraz_dyn->pixel;
    int avg;


  for (int y = 0; y<obraz_dyn->wymy; y++){
      for(int x = 1; x<obraz_dyn->wymx*obraz_dyn->typ-1; x+=3){
        avg = (piksele[y][x-1]+piksele[y][x]+piksele[y][x+1])/3;
        piksele[y][x-1]=piksele[y][x]=piksele[y][x+1]=avg;
      }
  }
  return zmiany[6] = 1;
}