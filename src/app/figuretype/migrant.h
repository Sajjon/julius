#ifndef FIGURETYPE_MIGRANT_H
#define FIGURETYPE_MIGRANT_H

#include "app/building/building.h"
#include "app/figure/figure.h"

void figure_create_immigrant(building *house, int num_people);

void figure_create_emigrant(building *house, int num_people);

void figure_create_homeless(int x, int y, int num_people);

void figure_immigrant_action(figure *f);

void figure_emigrant_action(figure *f);

void figure_homeless_action(figure *f);

#endif // FIGURETYPE_MIGRANT_H
