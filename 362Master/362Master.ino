int* nums = nullptr;
int* guessedNums = nullptr;
bool isStarted = true;
bool yetToGuess = true;
bool waitForGuess = false;

int roundNum = 1;
int lives = 3;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  randomSeed(analogRead(4));
}

void buildNums(){
  delete[] nums;
  nums = new int[roundNum];
  for(int i = 0; i < roundNum;++i){
    //Fill nums with random numbers 0-9 up to roundNum times
    nums[i] = random(10);
  }
}

void checkNums(){
  for(int i = 0; i < roundNum; ++i){
    if(guessedNums[i] != nums[i]){
      lives--;
      //Send out failed message
      if(lives < 1){
        //Send out you lose message
      }
    }
  }
}

void resetGame(){
  delete[] nums;
  delete[] guessedNums;
  lives = 3;
  roundNum = 1;
  isStarted = false;
  waitForGuess = false;
}


void loop() {
  // put your main code here, to run repeatedly:
  if(isStarted && yetToGuess){
    if(!waitForGuess){
      buildNums();
      waitForGuess = true;
    }else{
      //Wait for input, read in nums into guessedNums and handle successful logic
    }



  }
}
