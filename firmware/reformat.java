import java.io.*;
import java.util.*;


public class reformat {

 static public void main(String[] arg) {
  String thisLine = null;
  File file = new File("p25_data.txt");
  int mod=0;
          
    try {
      //BufferedReader br = new BufferedReader(new FileReader("p25_data.txt") );
			//BufferedReader br = new BufferedReader(new FileReader("pp_label_dibit.txt"));
			BufferedReader br = new BufferedReader(new FileReader("pp_label.txt"));
      
      while ((thisLine = br.readLine()) != null) {
				StringTokenizer st = new StringTokenizer(thisLine," \t");

				String laststr="";
				while(true) {
					try {
						laststr = st.nextToken();
					} catch(Exception e) {
						break;
					}
				}

        System.out.print(laststr+",");
        if(mod++>0 && mod%32==0) System.out.print("\n");
      }       

     } catch(Exception e) {
       e.printStackTrace();
     }
  }
}
