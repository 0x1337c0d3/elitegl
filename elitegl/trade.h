/*
 * trade.h
 */

#ifndef TRADE_H
#define TRADE_H
 
struct stock_item
{
	char name[16];
	int current_quantity;
	int current_price;
	int base_price;
	int eco_adjust;
	int base_quantity;
	int mask;
	int units;
};


#define NO_OF_STOCK_ITEMS	17
#define ALIEN_ITEMS_IDX		16

extern struct stock_item stock_market[NO_OF_STOCK_ITEMS];

void generate_stock_market (void);
void set_stock_quantities(int *quant);
int carrying_contraband (void);
int total_cargo (void);
void scoop_item (int un);

#endif

