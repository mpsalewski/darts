/******************************************************************************
 *
 * $FILE_NAME$.c/cpp
 *
 * 
 * $Project Name / Short Description / Titel$
 * 
 * 
 * $Subtitel / Important Notes$
 *
 *
 * author(s):   	$AuthorName$ <$e-mail$>, 
 *					$AuthorName$ <$e-mail$>, 	  
 * created on :     202X-YY-ZZ
 * last revision :  None
 *
 *
 *
 * Copyright (c) 202X, $AuthorName$, $AuthorName$ 
 * Version: 202X.YY.ZZ
 * License: CC BY-NC-SA 4.0,
 *      see https://creativecommons.org/licenses/by-nc-sa/4.0/deed.en
 *
 * 
 * Further information about this source-file:
 *      None
******************************************************************************/

/* compiler settings */


/***************************** includes **************************************/
#include "$FILE_NAME$.h"


/****************************** namespaces ***********************************/


/*************************** local Defines ***********************************/


/************************** local Structure **********************************/
struct module_name_s {
	int x = 0;
};
static struct module_name_s module_name = {
	0
};
/* optional: create ptr for access and compatibility */
static struct module_name_s* p = &module_name;

/************************* local Variables ***********************************/


/************************** Function Declaration *****************************/


/************************** Function Definitions *****************************/
/***
 *
 * func(void)
 *
 * About this function ...
 *
 *
 * @param:	void
 *          No parameters are required for this function
 *
 *
 * @return: void
 *
 *
 * @note:	None
 *
 *
 * Example usage: None
 *
***/
void func(void) {
	p->x = 1;
}