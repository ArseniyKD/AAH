
void pregame(){
	//one joystick sends constantly LRUDP values. if S is received, end function

}

void game(){
	//the game is going on.
}

void endScr(){
	//end screen, when receives R, end function
}

void GameMachine(){
	enum State {INIT, ACK, PREGAME, GAME, ENDSCREEN};
	State currentState = INIT;

	while(true){
		if(currentState == INIT){
			if(Serial3.available() != 0){
				int byteRead = Serial3.read();
				if(byteRead == 65){
					currentState = ACK;
				}
			}
		}
		if(currentState == ACK){
			Serial3.write((char)65);
			currentState = PREGAME;
		}

		if(currentState == PREGAME){
			pregame();
			currentState = GAME;
		}
		if(currentState == GAME){
			game();
			currentState = ENDSCREEN;
		}
		if(currentState == ENDSCREEN){
			endScr();
			currentState = INIT;
		}

	}
}

void menu(){

}
void scoreboard(){

}
void resetScreen(){
	
}


void ScoreMachine(){
	enum State {INIT, MENU, SCOREBOARD, ENDSCREEN};
	State currentState = INIT;

	while(true){
		if(currentState == INIT){
			Serial3.write((char)65);
			if(Serial3.available() != 0){
				int byteRead = Serial3.read();
				if(byteRead == 65){
					currentState = MENU;
				}
			}
		}
		if(currentState == MENU){
			menu();
			currentState = SCOREBOARD;
		}
		if(currentState == SCOREBOARD){
			scoreboard();
			currentState = ENDSCREEN;
		}
		if(currentState == ENDSCREEN){
			resetScreen();
			currentState = INIT;
		}
	}
}
