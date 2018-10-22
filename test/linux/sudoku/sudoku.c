#include<stdio.h>

int question[9][9];

int check (int count)
{
  int row=count/9;
  int col=count%9;
  int row2=row/3*3;
  int col2=col/3*3;
  int i,j,m,n;
  for(i=0;i<9;i++)
    if(question[row][col]==question[row][i]&&col!=i)
      return 0;
  for(j=0;j<9;j++)
    if(question[row][col]==question[j][col]&&row!=j)
      return 0;
  for(m=row2;m<row2+3;m++) {
    for(n=col2;n<col2+3;n++)
      if(question[row][col]==question[m][n]&&row!=m&&col!=m)
        return 0;
  }
  return 1;
}

void output(int count)
{
  int i,j,x;
  int q,p;
  int row=count/9;
  int col=count%9;

  if(count==81) {
    printf("answer:\n");
    for(q=0;q<9;q++) {
      for(p=0;p<9;p++)
        printf("%d ",question[q][p]);
      printf("\n");
    }
    return;
  }
  if(question[row][col]==0) {
    for(i=1;i<10;i++) {
      question[row][col]=i;
      if(check(count)==1)
        output(count+1);
      question[row][col]=0;
    }
  }
  else {
    output(count+1);
  }
}

int main()
{
  int i,j,count=0;
  printf("input a 9x9 matrix: set 0 to be the unknown number.\n");
  printf("an example:\n");
  printf("0 3 4 6 7 8 9 1 2\n");
  printf("6 0 2 1 9 5 3 4 8\n");
  printf("1 9 0 3 4 2 5 6 7\n");
  printf("8 5 9 0 6 1 4 2 3\n");
  printf("4 2 6 8 0 3 7 9 1\n");
  printf("7 1 3 9 2 0 8 5 6\n");
  printf("9 6 1 5 3 7 0 8 4\n");
  printf("2 8 7 4 1 9 6 0 5\n");
  printf("3 4 5 2 8 6 1 7 0\n\n");
  for(i=0;i<9;i++) {
    for(j=0;j<9;j++)
      scanf("%d", &question[i][j]);
  }
  output(count);
}
