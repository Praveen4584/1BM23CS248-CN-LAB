#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main()
{
    char rem[50], a[100], s[100], c, msj[100], gen[50];
    int i, genlen, t, j, flag = 0, k, n;

    printf("Enter the generator polynomial:\n");
    fgets(gen, sizeof(gen), stdin);
    gen[strcspn(gen, "\n")] = '\0';   // remove newline

    printf("Generator polynomial (CRC-CCITT): %s\n", gen);

    genlen = strlen(gen);
    k = genlen - 1;

    printf("Enter the message:\n");

    n = 0;
    while ((c = getchar()) != '\n' && c != EOF)
    {
        msj[n] = c;
        n++;
    }
    msj[n] = '\0';

    // Append zeros
    for (i = 0; i < n; i++)
        a[i] = msj[i];

    for (i = 0; i < k; i++)
        a[n + i] = '0';

    a[n + k] = '\0';

    printf("\nMessage polynomial appended with zeros:\n");
    puts(a);

    // Division process
    for (i = 0; i < n; i++)
    {
        if (a[i] == '1')
        {
            t = i;
            for (j = 0; j <= k; j++)
            {
                a[t] = (a[t] == gen[j]) ? '0' : '1';
                t++;
            }
        }
    }

    for (i = 0; i < k; i++)
        rem[i] = a[n + i];

    rem[k] = '\0';

    printf("The checksum is:\n");
    puts(rem);

    printf("\nMessage with checksum appended:\n");

    for (i = 0; i < n; i++)
        a[i] = msj[i];

    for (i = 0; i < k; i++)
        a[n + i] = rem[i];

    a[n + k] = '\0';

    puts(a);

    // Receiver side
    n = 0;
    printf("Enter the received message:\n");

    while ((c = getchar()) != '\n' && c != EOF)
    {
        s[n] = c;
        n++;
    }
    s[n] = '\0';

    for (i = 0; i < n - k; i++)
    {
        if (s[i] == '1')
        {
            t = i;
            for (j = 0; j <= k; j++)
            {
                s[t] = (s[t] == gen[j]) ? '0' : '1';
                t++;
            }
        }
    }

    for (i = 0; i < k; i++)
        rem[i] = s[n - k + i];

    rem[k] = '\0';

    for (i = 0; i < k; i++)
    {
        if (rem[i] == '1')
            flag = 1;
    }

    if (flag == 0)
        printf("Received polynomial is error-free.\n");
    else
        printf("Received polynomial has errors!\n");

    return 0;
}
