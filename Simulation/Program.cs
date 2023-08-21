using System.Reflection.PortableExecutable;

public class Program
{
    static void Main(string[] args)
    {
        Random rnd = new();
        int lastNum = 80;
        int num = 0;

        while(true)
        {
            int change = rnd.Next(101);

            if(change >= 0 && change <= 10)
            {
                num = lastNum - 1;
                lastNum --;
            }else if(change > 10 && change <= 20)
            {
                num = lastNum + 1;
                lastNum ++;
            }else if(change > 20 && change <= 93)
            {
                num = lastNum;
            }else
            {
                change = rnd.Next(21);
                if(change >= 0 && change < 17)
                {
                    num = 41;
                }else{
                    num = 185;
                }
            }

            Console.WriteLine($"{num}%");
            Thread.Sleep(500);
        }
    }
}