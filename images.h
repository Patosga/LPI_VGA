#ifndef __IMAGES_H_
#define __IMAGES_H_

#define TRANSPARENCE 0xFF
#define strings_total 2
extern const unsigned char *image_vector[];
extern char *words[strings_total];
extern const unsigned char *ascii_vector[];


void image_vector_init(void);
void ascii_vector_init(void);
#endif
