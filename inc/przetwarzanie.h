int czytaj(FILE *, obraz_t *);
int zapisz(FILE *, obraz_t *, char *, int *, char *, float *, int);
void wyswietl(obraz_t *, t_opcje*, int *, char *, float *, int);
int negatyw(obraz_t *, int *);
int progowanie(obraz_t *, int, int *, int *);
int korekcja_gamma(obraz_t *, float, int *, int *);
int konturowanie(obraz_t *, int *);
int histogram(obraz_t *, int *);
int konwersa(obraz_t *, int *);