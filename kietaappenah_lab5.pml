int n;
int endcount;
proctype processp()
{
	byte temp, count = 2 ; 
	do :: (count == 0) -> break
	   :: else ->
		  temp = n; 
		  temp++;
		  n = temp;
		  count-- ; 	  
	od;
	endcount++;
}

proctype processq()
{
	byte temp, count = 2 ; 
	do :: (count == 0) -> break
	   :: else ->
		  n++; 
		  count-- ; 		  
	od;
	endcount++;
}

init
{	
	n = 0;
	endcount=0;
	run processp(); 
	run processq();
	printf("Count: %d \n", n);
	endcount=2;
	assert(n!=3);
}
