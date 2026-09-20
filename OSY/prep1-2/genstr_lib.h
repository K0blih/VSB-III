#ifndef GENSTR_LIB_H
#define GENSTR_LIB_H

/* Vraci alokovany radek bez '\n'; volajici ho musi uvolnit pomoci free().
 * max_words musi byt kladne. Pri chybe vraci NULL. */
char *generate_line(int max_words);

#endif
