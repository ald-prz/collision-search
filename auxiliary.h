#ifndef AUXILIARY_H
#define AUXILIARY_H

unsigned char mask[8] = {1, 3, 7, 15, 31, 63, 127, 255};

/*!
 * \brief IsMatch Checks partial match of two binary words (assumes sizes of words = 20 bytes)
 * \param one First word
 * \param two Second word
 * \param number_of_bits Number of least significant bits determing the match
 * \return 1 if matching, 0 otherwise
 */
int is_match(unsigned char *one, unsigned char *two, int number_of_bits)
{
    const int size = 20;
    int number_of_bytes = number_of_bits / 8;
    int extra_bits = number_of_bits - 8 * number_of_bytes;
    unsigned char first_byte;
    unsigned char second_byte;

    int i;

    for (i = size - 1; i >= size - number_of_bytes; i--)
        if (one[i] != two[i])
            return 0;

    if (number_of_bits != 0)
    {
        first_byte = one[size - number_of_bytes - 1] & mask[extra_bits - 1];
        second_byte = two[size - number_of_bytes - 1] & mask[extra_bits - 1];

        if (first_byte != second_byte)
            return 0;
    }

    return 1;
}

int full_match(unsigned char *one, unsigned char *two, int number_of_bytes)
{
    int i;

    for (i = 0; i < number_of_bytes; i++)
        if (one[i] != two[i])
            return 0;

    return 1;
}

/*!
 * \brief increment Increments given binary word
 * \param word Word
 * \param length Lenght of the given word in bytes
 */
void increment(unsigned char *word, int length)
{
    int i;
    int nextFlag = 0;

    word[length - 1]++;
    if (word[length - 1] == 0)
        nextFlag = 1;

    for (i = length - 2; i >= 0; i--)
        if (nextFlag == 1)
        {
            word[i]++;
            if (word[i] != 0)
                nextFlag = 0;
        }
        else
            break;
}

#endif // AUXILIARY_H
