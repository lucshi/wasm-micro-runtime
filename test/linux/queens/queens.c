#include<stdio.h>

int abs(int a) {
  if (a < 0)
    a = -a;
  return a;
}

int max=8,sum=0,a[8];

void show()
{
  for(int i=0;i<max;i++) {
    printf("(%d,%d)\t",i,a[i]);
  }
  printf("\n");
}

int check(int n)
{
  for(int i=0;i<n;i++){
    if(a[i]==a[n]||abs(a[n]-a[i])==(n-i))
      return 0;
  }
  return 1;
}

void eightQueen(int n)
{
  int i;
  if(n<max) {
    for(i=0;i<max;i++) {
      a[n]=i;
      if(check(n))
        eightQueen(n+1);
    }
  }
  else {
    sum++;
    show();
  }
}

int main()
{
  eightQueen(0);
  printf("%d\n",sum);
}
