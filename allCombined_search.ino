/*
 * 
 * 
 * 
 * 4. SEARCH FUNCTION
 * 
 * 
 * 
 * 
 * 
 * 
 */
//////////////////////////////////////////////////////
//Search function
int shortestDist(int startPos, int facing) {
  int min_cost=100;
  int min_dest_i=0;
  for (int i=0; i<(sizeof(DEST)/sizeof(DEST[0])); i++) {
    if (DEST[i]>=0) {
      int dist_x=DEST[i]/10-startPos/10;
      int dist_y=DEST[i]%10-startPos%10;
      int cost_to_dest=abs(dist_x)+abs(dist_y);
      
      //if turning occur in journey middle
      if (dist_x!=0 and dist_y!=0) {
        cost_to_dest++;
      }
      
      //if turning at starting
      if (facing==0 and dist_x<=0)	{
        cost_to_dest++;
      }
      if (facing==1 and dist_y<=0)	{
        cost_to_dest++;
      }
      if (facing==2 and dist_x>=0)	{
        cost_to_dest++;
      }
      if (facing==3 and dist_y>=0)	{
        cost_to_dest++;
      }
      
      if (cost_to_dest < min_cost)  {
        min_cost = cost_to_dest;
        min_dest_i = i;
      }
    }
  }
  int min_dest=DEST[min_dest_i];
  DEST[min_dest_i]=-1; //i.e. remove that element
  return min_dest;
}

void findRouteUCS(int destPos, int routeUCS[]) {
  int frontier[(SIZE+1)*(SIZE+1)][2];
  int prev_pos[(SIZE+1)*(SIZE+1)][2];
  int prev_pos_facing[(SIZE+1)*(SIZE+1)];
  arrInitToSize2D(frontier); //init to -1
  arrInitToSize2D(prev_pos);
  arrInitToSize(prev_pos_facing);
  
  frontier[0][0]=0;       //cumulative cost
  frontier[0][1]=NowPos;  //pos
  
  prev_pos[0][0]=NowPos;  //start from
  prev_pos[0][1]=-1;      //its prev pos. (=NONE)
  prev_pos_facing[0]=Facing;  //facing
  
  while (!isArrEmpty2D(frontier,0)) { //while frontier not empty
    int minCostIndex=findMinCostIndex(frontier);
    int prev_cost=frontier[minCostIndex][0];
    int now_pos=frontier[minCostIndex][1];
    int now_facing=prev_pos_facing[elementIndexinArray2D(now_pos,prev_pos,0)];
    frontier[minCostIndex][0]=-1; //remove min cost frontier
  
    int neighbour[]={-1,-1,-1,-1};
    int moving_cost[]={-1,-1,-1,-1};
    int next_facing[]={-1,-1,-1,-1};
    findNeighbour(now_pos,now_facing,neighbour,moving_cost,next_facing);

    for (int i=0; i<4; i++) {
      if (neighbour[i]>=0) {//valid neighbour
        int next_cost=prev_cost+moving_cost[i];
        //cost to neighbour less than another route or neighbour is not prev pos 
        if (next_cost<findElementinArray2D(neighbour[i],frontier,1,0) or !isElementinArray2D(neighbour[i],prev_pos,0)) {
          arrayAppend2D(frontier,next_cost,neighbour[i]);
          //update prev_pos
          if (isElementinArray2D(neighbour[i], prev_pos,0)) { 
            int j=elementIndexinArray2D(neighbour[i],prev_pos,0);
            prev_pos[j][0]=neighbour[i];
            prev_pos[j][1]=now_pos;
            prev_pos_facing[j]=next_facing[i];
          } else {
            arrayAppend2D(prev_pos,neighbour[i],now_pos);
            arrayAppend(prev_pos_facing,next_facing[i]);
          }
        }
      }
    }
  }

  //linking dest in prev_pos to start pos
  arrInitToSize(routeUCS);
  routeTraceback(routeUCS,NowPos,destPos,prev_pos);
}

void findRoute(int destPos) {
  int now_x=NowPos/10;
  int now_y=NowPos%10;
  int dest_x=destPos/10;
  int dest_y=destPos%10;
  int dist_x=dest_x-now_x;
  int dist_y=dest_y-now_y;
  
  //if no turning is needed
  if (now_x==dest_x or now_y==dest_y) {
    Route[0]=10*dest_x+dest_y;
    Route[1]=-1;
  }
  else {
    int tpx=0; //pos of turning pt.
    int tpy=0;
    
    //dest in forward
    if ((Facing==0 and dist_x>0) or (Facing==2 and dist_x<0))	{
      tpx=dest_x;
      tpy=now_y;
    }
  	if ((Facing==1 and dist_y>0) or (Facing==3 and dist_y<0))	{
      tpx=now_x;
      tpy=dest_y;
    }
  	
  	//dest in backward
  	if ((Facing==0 and dist_x<0) or (Facing==2 and dist_x>0))	{
      tpx=now_x;
      tpy=dest_y;
    }
  	if ((Facing==1 and dist_y<0) or (Facing==3 and dist_y>0))	{
      tpx=dest_x;
      tpy=now_y;
    }
      
    Route[0]=10*tpx+tpy;
    Route[1]=10*dest_x+dest_y;
  }
}

void posForwardUpdate() {
  switch (Facing) {
    case 0: //+x 
      NowPos+=10;
      break;
    case 1: //+y
      NowPos+=1;
      break;
    case 2: //-x
      NowPos-=10;
      break;
    case 3: //-y
      NowPos-=1;
      break;
  }
  if (task_sel==3) {
    DEST[elementIndexinArray(NowPos,DEST)]=-1;
  }
}

void routeShorten(int arr[], int arrReturn[]) {
  arrInitToSize(arrReturn);
  if (arr[0]!=-1) {    
    int chng1=NowPos-arr[0];
    int chng2=arr[0]-arr[1];
    for (int i=0; i<(SIZE+1)*(SIZE+1)-2; i++) {
      if (chng1!=chng2) {
        arrayAppend(arrReturn,arr[i]);
      }
      chng1=arr[i]-arr[i+1];
      chng2=arr[i+1]-arr[i+2];
    }
  }
}
//////////////////////////////////////////////////////
//UCS related function
int routeTraceback(int route[], int start_pos, int dest_pos, int prev_pos[][2]) {  
  if (isElementinArray2D(dest_pos,prev_pos,0)) {
    int now_pos=dest_pos;
    while (now_pos!=start_pos) {
      arrayAppend(route,now_pos);
      now_pos=findElementinArray2D(now_pos,prev_pos,0,1);
    }
    arrayReverse(route);
  }
}

int arrayReverse(int arr[]) {
  int i=0;
  int j=elementIndexinArray(-1,arr)-1;
  int temp;
  while (i<j) {
    temp=arr[i];
    arr[i]=arr[j];
    arr[j]=temp;
    i++;
    j--;
  }
}

int findMinCostIndex(int arr[][2]) {
  int n=(SIZE+1)*(SIZE+1);
  int temp;
  int min_i=0;
  while (arr[min_i][0]<0) {
    min_i++;
  }
  temp=arr[min_i][0];
  for(int i=min_i; i<n; i++) {
      if(temp>arr[i][0] and arr[i][0]>=0) { //-1 mean remove
         temp=arr[i][0];
         min_i=i;
      }
   }
   return min_i;
}


int findElementinArray2D(int element, int arr[][2], int rowFind, int rowReturn) {
  int n=(SIZE+1)*(SIZE+1);
  for (int i=0; i<n ;i++) {
    if (element==arr[i][rowFind]) {
      return arr[i][rowReturn];
    }
  }
  return -1; //not find
}

int elementIndexinArray(int element,int arr[]) {
  int n=(SIZE+1)*(SIZE+1);
  for (int i=0; i<n ;i++) {
    if (element==arr[i]) {
      return i;
    }
  }
  return -1; //not find
}

int elementIndexinArray2D(int element, int arr[][2], int rowFind) {
  int n=(SIZE+1)*(SIZE+1);
  for (int i=0; i<n ;i++) {
    if (element==arr[i][rowFind]) {
      return i;
    }
  }
  return -1; //not find
}

int isElementinArray(int element, int arr[]) {
  int n=(SIZE+1)*(SIZE+1);
  for (int i=0; i<n; i++) {
    if (element==arr[i]) {
      return true;
    }
  }
  return false;
}

int isElementinArray2D(int element, int arr[][2], int rowFind) {
  int n=(SIZE+1)*(SIZE+1);
  for (int i=0; i<n; i++) {
    if (element==arr[i][rowFind]) {
      return true;
    }
  }
  return false;
}

void arrayAppend(int arr[], int element) {
  int i=0;
  while(i<(SIZE+1)*(SIZE+1) and arr[i]>=0) { 
    i++;   
  }
  if (arr[i]<0) {
    arr[i]=element;
  }
}

void arrayAppend2D(int arr[][2], int element1, int element2) { 
  int i=0;
  while(i<(SIZE+1)*(SIZE+1) and arr[i][0]>=0) { 
    i++;   
  }
  if (arr[i][0]<0) {
    arr[i][0]=element1;
    arr[i][1]=element2;
  }
}

void arrInitToSize(int arr[]) {
  for (int i=0; i<(SIZE+1)*(SIZE+1); i++) {
    arr[i]=-1;
  }
}

void arrInitToSize2D(int arr[][2]) {
  for (int i=0; i<(SIZE+1)*(SIZE+1); i++) {
    for (int j=0; j<2; j++) {
      arr[i][j]=-1;
    }
  }
}

bool isArrEmpty2D(int arr[][2], int rowFind) {
  for (int i=0; i<(SIZE+1)*(SIZE+1); i++) {
    if (arr[i][rowFind]!=-1) {
      return false;
    }
  }
  return true;
}

void findNeighbour(int pos, int facing, int neighbour[], int moving_cost[], int next_facing[]) {
  int temp[]={10,1,-10,-1}; //x++, y++, x--, y--
  for (int i=0; i<4; i++) {
    int next_pos=pos+temp[i];
    if (isValidPos(next_pos) and next_pos>=0) {      
      int cost=abs(facing-i);
      if (cost==3) {
        cost=1;
      }
      neighbour[i]=next_pos;
      moving_cost[i]=cost+1; //moving 1 grid and turning cost
      next_facing[i]=i;
    }
  }
}

bool isValidPos(int pos) { //check within the map and not obstacle
  if (pos>=0 and (pos/10<=SIZE and pos%10<=SIZE)) {
    if (isElementinArray(pos,Obstacle)) {
      return false;
    } else {
      return true;
    }
  } else {
    return false;
  }
}

/////////////////////////////////////////////////////////////
