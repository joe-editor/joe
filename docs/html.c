/* Convert text file into html */

main()
  {
  char buf[1024];
  int inpara = 0;
  while(gets(buf))
    {
    int x;
    int isblank = 1;
    for (x = 0; buf[x]; ++x)
      if(buf[x] != ' ' && buf[x] != '\t')
        isblank = 0;
    if (!inpara)
      {
      if (isblank)
        {
        printf("\n");
        }
      else
        {
        inpara = 1;
        printf("<p>%s\n",buf);
        }
      }
    else
      {
      if (isblank)
        {
        printf("</p>\n");
        inpara = 0;
        }
      else
        {
        printf("%s\n",buf);
        }
      }
    }
  }
