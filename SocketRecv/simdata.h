#include <stdlib.h>
#include "socketincludes.h"
#include "config.h"
#include <math.h>

// External function to retrieve random data from composers
// since we focus on heartrate monitors somewhat resembeling those datasets
int getRandomData()
{
    int randData = random();
    randData = 85 + (randData % 40);
    return randData;
}