#ifndef DIGEST_MATCH_H
#define DIGEST_MATCH_H

/*!
 * \brief IsMatch Checks partial match of two binary words (assumes sizes of words = 20 bytes)
 * \param one First word
 * \param two Second word
 * \param number_of_bits Number of least significant bits determing the match
 * \return 1 if matching, 0 otherwise
 */
int digest_match(unsigned char *one, unsigned char *two, int number_of_bits);

#endif // DIGEST_MATCH_H
