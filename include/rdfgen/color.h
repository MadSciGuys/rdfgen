// Travis Whitaker 2013-2015
// twhitak@its.jnj.com

#ifndef COLOR_H
#define COLOR_H

#ifndef NO_COLOR

#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define YELLOW "\x1b[33m"
#define BLUE "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN "\x1b[36m"
#define BOLD "\x1b[1m"
#define UNDERLINE "\x1b[4m"
#define REVERSE "\x1b[7m"
#define BLINK "\x1b[5m"
#define BLINK_EXPERIMENTAL "\x1b[6m"
#define RESET "\x1b[0m"

#else

#define RED 
#define GREEN
#define YELLOW
#define BLUE
#define MAGENTA
#define CYAN
#define BOLD
#define UNDERLINE
#define REVERSE
#define BLINK
#define BLINK_EXPERIMENTAL
#define RESET

#endif


#endif
