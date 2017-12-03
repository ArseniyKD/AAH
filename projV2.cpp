#include <Arduino.h>
#include<Adafruit_ILI9341.h>
#include <math.h> //M_PI is pi


#define TFT_DC 9
#define TFT_CS 10
#define SD_CS 6

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

#define DISPLAY_WIDTH  320
#define DISPLAY_HEIGHT 240

#define JOY_VERT  A1
#define JOY_HORIZ A0
#define JOY_SEL   2

#define JOY_CENTER   512
#define JOY_DEADZONE 32

#define PUCK_RAD 8
#define MALLET_RAD 12
#define COLLISION_DIST 20


float collision_X, collision_Y;

struct Circle{
	float xCoord;
	float yCoord;
	float xVelocity;
	float yVelocity;
};

Circle Puck;
Circle Mallet1;
int speedLimit = 11;
int speedLimitM = 8;



void setup(){
	init();

	Serial.begin(9600);
	tft.begin();

	tft.setRotation(3);
	tft.fillScreen(ILI9341_BLACK);

	Puck.xCoord = DISPLAY_WIDTH / 2;
	Puck.yCoord = DISPLAY_HEIGHT / 2;

	tft.fillCircle(Puck.xCoord, Puck.yCoord, PUCK_RAD, ILI9341_RED);

	Puck.xVelocity = 10;
	Puck.yVelocity = 10;

	Mallet1.xCoord = 25;
	Mallet1.yCoord = DISPLAY_HEIGHT / 2;

	tft.fillCircle(Mallet1.xCoord, Mallet1.yCoord, MALLET_RAD, ILI9341_BLUE);
}



void PuckMovement(){

	tft.fillCircle(Puck.xCoord, Puck.yCoord, PUCK_RAD, ILI9341_BLACK);

	Puck.xCoord += Puck.xVelocity;
	Puck.yCoord += Puck.yVelocity;

	if(Puck.xCoord - PUCK_RAD + Puck.xVelocity <= 0 || Puck.xCoord + PUCK_RAD + Puck.xVelocity >= DISPLAY_WIDTH){
		Puck.xVelocity *= -0.95;
	}
	if(Puck.yCoord - PUCK_RAD + Puck.yVelocity <= 0 || Puck.yCoord + PUCK_RAD + Puck.yVelocity >= DISPLAY_HEIGHT){
		Puck.yVelocity *= -0.95;
	}

	Puck.xCoord = constrain(Puck.xCoord, 0 + PUCK_RAD, DISPLAY_WIDTH - PUCK_RAD);
	Puck.yCoord = constrain(Puck.yCoord, 0 + PUCK_RAD, DISPLAY_HEIGHT - PUCK_RAD);

	tft.fillCircle(Puck.xCoord,Puck.yCoord, PUCK_RAD, ILI9341_RED);
	delay(20);

}

void correctPuckLocation(float distance){
	if(distance < COLLISION_DIST){
		tft.fillCircle(Puck.xCoord, Puck.yCoord, PUCK_RAD, ILI9341_BLACK);
		if(Mallet1.xCoord <= Puck.xCoord){
			Puck.xCoord += COLLISION_DIST - distance;
		}
		if(Puck.xCoord < Mallet1.xCoord){
			Puck.xCoord -= COLLISION_DIST - distance;
		}
		if(Mallet1.yCoord < Puck.yCoord){
			Puck.yCoord += COLLISION_DIST - distance;
		}
		if(Puck.yCoord < Mallet1.yCoord){
			Puck.yCoord -= COLLISION_DIST - distance;
		}

	}

}

void collisionPoint(){
	collision_X = ((Mallet1.xCoord*PUCK_RAD)+(Puck.xCoord*MALLET_RAD))/(COLLISION_DIST);
	collision_Y = ((Mallet1.yCoord*PUCK_RAD)+(Puck.xCoord*MALLET_RAD))/(COLLISION_DIST);
}

// void VelocityUpdate(){
//
// }

void simpleCollisionReflection(){
	// if(Mallet1.xCoord < Puck.xCoord){
	// 	if(Mallet1.yCoord < Puck.yCoord){ //4th quad
	// 		Puck.yVelocity *= -1;
	// 	}
	// 	else if(Mallet1.yCoord > Puck.yCoord){ //3rd quad
	// 		Puck.
	// 	}
	// }
	// else if(Mallet1.xCoord > Puck.xCoord){
	// 	if( Mallet1.yCoord < Puck.yCoord){//2nd quad
	// 		angleOfContact = (PI/2) - calcAngleOfContact();
	// 	}
	// 	else if (Mallet1.yCoord > Puck.xCoord){//1st quad
	// 		angleOfContact = (PI /2) - calcAngleOfContact();
	// 	}
	// }

	if(Mallet1.yCoord == Puck.yCoord){

		Puck.xVelocity *= -1;
	}
	else{
		Puck.yVelocity *= -1;
	}


}

void velocityDirectionBasedReflection(){
	if(Mallet1.yCoord == Puck.yCoord){

		Puck.xVelocity *= -1;
		Puck.yVelocity += Mallet1.yVelocity;
		Puck.xVelocity += Mallet1.xVelocity;
	}
	else{
		if(Puck.yVelocity < 0){
			Puck.xVelocity *= -1;
			Puck.yVelocity += Mallet1.yVelocity;
			Puck.xVelocity += Mallet1.xVelocity;
		}
		else{
			Puck.yVelocity *= -1;
			Puck.yVelocity += Mallet1.yVelocity;
			Puck.xVelocity += Mallet1.xVelocity;
		}
	}
}


float getDist(float X1, float X2, float Y1, float Y2){
	return sqrt(pow(X1 - X2, 2) + pow(Y1 - Y2, 2));
}

void vectorReflection() {

	int puckMass = 0.1;
	int malletMass = 1;
	float restitution = 1;

	float n[2] = {abs(Puck.xCoord - Mallet1.xCoord), Puck.yCoord - Mallet1.yCoord};
	// for(int i = 0; i <2; i++){
	// 	Serial.print("n values: ");
	// 	Serial.println(n[i]);
	// }

	//find the unit vector for the normal line
  	float un[2]	 = {n[0]/sqrtf((n[0]*n[0])+(n[1]*n[1])), n[1]/sqrtf((n[0]*n[0])+(n[1]*n[1]))};
	// for(int i = 0; i <2; i++){
	// 	Serial.print("un values: ");
	// 	Serial.println(un[i]);
	// }

	//no longer need vector n, only un, can dealocate n if necessary

	//find the unit vector for the line tangent to the collision
	float ut[2] = {-un[1], un[0]};

	//compute un dot product velocity to find normal component initially
	float vPuckin = Puck.xVelocity*un[0] + Puck.yVelocity*un[1];

	float vMallet1in = Mallet1.xVelocity*un[0] + Mallet1.yVelocity*un[1];

	//compute ut dot product velocity to find tangential component which is constant before and after collision
	float vPuckt = Puck.xVelocity*ut[0] + Puck.yVelocity*ut[1];

	float vMallet1t = Mallet1.xVelocity*ut[0] + Mallet1.yVelocity*ut[1];

	//compute normal component after collision using formula from http://www.vobarian.com/collisions/2dcollisions2.pdf
	float vPuckfn = (vPuckin*(puckMass-malletMass)+2*malletMass*vMallet1in)/(puckMass + malletMass);

	float vMalletfn = (vMallet1in*(malletMass-puckMass)+2*puckMass*vPuckin)/(puckMass + malletMass);

	//add the x or y portions of the normal and tangential velocities
	Puck.xVelocity = constrain((vPuckfn*un[0] + vPuckt*ut[0])*restitution, -speedLimit, speedLimit);
	Puck.yVelocity = constrain((vPuckfn*un[1] + vPuckt*ut[1])*restitution, -speedLimit, speedLimit);

	Mallet1.xVelocity = (vMalletfn*un[0] + vMallet1t*un[0])*restitution;
	Mallet1.yVelocity = (vMalletfn*un[1] + vMallet1t*un[1])*restitution;

}

void checkForCollision(){


	float distance = getDist(Mallet1.xCoord, Puck.xCoord, Mallet1.yCoord, Puck.yCoord);

	if(distance <= COLLISION_DIST){

		correctPuckLocation(distance);

		//collisionPoint();

		//float angleOfContact;

		// if(Mallet1.xCoord < Puck.xCoord){
		// 	if(Mallet1.yCoord < Puck.yCoord){ //4th quad
		// 		angleOfContact = (PI / 2) - calcAngleOfContact();
		// 	}
		// 	else if(Mallet1.yCoord > Puck.yCoord){ //3rd quad
		// 		angleOfContact = (PI/2) - calcAngleOfContact();
		// 	}
		// }
		// else if(Mallet1.xCoord > Puck.xCoord){
		// 	if( Mallet1.yCoord < Puck.yCoord){//2nd quad
		// 		angleOfContact = (PI/2) - calcAngleOfContact();
		// 	}
		// 	else if (Mallet1.yCoord > Puck.xCoord){//1st quad
		// 		angleOfContact = (PI /2) - calcAngleOfContact();
		// 	}
		// }
		// Puck.xVelocity = Puck.xVelocity - ((((0.2)/1.1)*((Puck.xVelocity - Mallet1.xVelocity))) / pow(distance,2));
		// Puck.yVelocity = Puck.yVelocity - ((((0.2)/1.1)*((Puck.yVelocity - Mallet1.yVelocity))) / pow(distance,2));
		//simpleCollisionReflection();
		vectorReflection();

	}

}

bool antiFlicker(int changeInX, int changeInY){
	if(changeInX < JOY_CENTER - JOY_DEADZONE || changeInX > JOY_CENTER + JOY_DEADZONE
	|| changeInY > JOY_CENTER + JOY_DEADZONE || changeInY < JOY_CENTER - JOY_DEADZONE){
		return true;
	}
	else{
		Mallet1.xVelocity = 0;
		Mallet1.yVelocity = 0;
		return false;
	}
}

void MalletMovement(){


	int xVal = analogRead(JOY_HORIZ);
    int yVal = analogRead(JOY_VERT);

	if(antiFlicker(xVal, yVal)){

		tft.fillCircle(Mallet1.xCoord, Mallet1.yCoord, MALLET_RAD, ILI9341_BLACK);

		if(yVal < JOY_CENTER - JOY_DEADZONE){
	      //move the cursor up with a speed proportional to yVal
	      Mallet1.yCoord -= constrain((JOY_CENTER - yVal) / 40, -speedLimitM, speedLimitM);
		  Mallet1.yVelocity = constrain((yVal - JOY_CENTER) / 40, -speedLimitM, speedLimitM);
	    }

	    //if the joystick has been pushed down
	    else if(yVal > JOY_CENTER + JOY_DEADZONE){
	      //move the cursor down with a speed proportional to yVal
	      Mallet1.yCoord += constrain((yVal - JOY_CENTER) / 40, -speedLimitM, speedLimitM);
		Mallet1.yVelocity = constrain((yVal - JOY_CENTER) / 40, -speedLimitM, speedLimitM);
	    }

	    //if the joystick has been moved left
	    if(xVal > JOY_CENTER + JOY_DEADZONE){
	      //move the cursor left with a speed proportional to xVal
	      Mallet1.xCoord -= constrain((xVal - JOY_CENTER)/40, -speedLimitM, speedLimitM);
		  Mallet1.xVelocity = constrain((JOY_CENTER - xVal)/40, -speedLimitM, speedLimitM);
	    }

	    //if the joystick has been moved right
	    else if( xVal < JOY_CENTER - JOY_DEADZONE){
	      //move the cursor right with a speed proportional to xVal
	      Mallet1.xCoord += constrain((JOY_CENTER - xVal)/40, -speedLimitM, speedLimitM);
		  Mallet1.xVelocity = constrain((JOY_CENTER - xVal)/40, -speedLimitM, speedLimitM);
	    }

		//constrain the location of the cursor to be within the screen dimensions
	    Mallet1.xCoord = constrain(Mallet1.xCoord, MALLET_RAD ,DISPLAY_WIDTH - MALLET_RAD);
	    //constrain the location of the cursor to be within the screen dimensions
	    Mallet1.yCoord = constrain(Mallet1.yCoord, MALLET_RAD ,DISPLAY_HEIGHT - MALLET_RAD);
	}

	tft.fillCircle(Mallet1.xCoord, Mallet1.yCoord, MALLET_RAD, ILI9341_BLUE);

}


int main(){
	setup();
	while(true){
		PuckMovement();
		MalletMovement();
		checkForCollision();
	}

	return 0;
}
