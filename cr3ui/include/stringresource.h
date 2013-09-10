/*
 * stringresource.h
 *
 *  Created on: Aug 21, 2013
 *      Author: vlopatin
 */

#ifndef STRINGRESOURCE_H_
#define STRINGRESOURCE_H_

#ifndef STRING_RES_IMPL
#define S(x) extern const char * STR_ ## x;
#else
#define S(x) const char * STR_ ## x  = # x;
#endif

S(NOW_READING)
S(BROWSE_FILESYSTEM)
S(BROWSE_LIBRARY)
S(ONLINE_CATALOGS)
S(RECENT_BOOKS)
S(SD_CARD_DIR)
S(INTERNAL_STORAGE_DIR)
S(BOOKS_BY_AUTHOR)
S(BOOKS_BY_TITLE)
S(BOOKS_BY_FILENAME)
S(BOOKS_BY_SERIES)
S(BOOKS_SEARCH)
S(BOOK_COUNT)
S(FOLDER_COUNT)


#endif /* STRINGRESOURCE_H_ */
