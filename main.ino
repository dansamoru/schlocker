#define section 6
#define deleteDelay 1500







class Locker{
  public:
    int userId = NULL;
    bool isUsed = false;
    //datetime lastAct;
    bool isOpened();
    void openDoor(){
      return 0;
    }
    void deleteUser(){
      return 0;
    }
};

Locker lockers[section];


void registration(int cardId){
  
}

bool availableCard(){
  Serial.print("Enter card availble (Y/N): ");
  while(Serial.available()==0);
  char s = Serial.read();
  if(s == 'Y'){
    return true;
  }else{
    return false;
  }
}

int cardWritten(int cardId){
  for(int i = 0; i<section;i++){
    if(lockers[i].userId == cardId){
      return i;
    }
  }
}

int *availableLockers(){
  int availableLockers[section];
  for(int i = 0; i < section; i++){
    if(lockers[i].isUsed == false){
      availableLockers[i] = i;
    }else{
      availableLockers[i]=NULL;
    }
  }
  return availableLockers;
}

bool notNull(int *arr, int s){
  for(int i = 0; i<s;i++){
    if(arr[i] != NULL){
      return true;
    }
  }
  return false;
}

bool buttonPressed(){
  Serial.print("Enter button position (Y/N): ");
  while(Serial.available()==0);
  char s = Serial.read();
  if(s == 'Y'){
    return true;
  }else{
    return false;
  }
}

void deleteChecker(int lockerId){
  int startTime = millis();
  while(millis() - startTime <= deleteDelay){
    if(buttonPressed()){
      lockers[lockerId].deleteUser();
      return 0;
    }
    if(availableCard()){
      return 0;
    }
  }
}
bool openLocker(int lockerId){
  lockers[lockerId].openDoor();
  deleteChecker(lockerId);

}

void cardInput(){
  if(availableCard){
    int cardLocker = cardWritten(228);
    if(cardLocker != NULL){
      openLocker(cardLocker);
      registration(228);
    }
  }
}


int main(){
  cardInput();
  return 0;
}

void setup(){
  Serial.begin(9600);
}

void loop(){
  main();
}
