#include <Arduino.h>
#include "RandomForestRegressor.h"

Eloquent::ML::Port::RandomForestRegressor regressor;
float X[] = {...};


void setup() {
    
}

void loop() {
    float y_pred = regressor.predict(X);
}
