import java.io.File;

public class PutRuptureVariationsInDB {
	private static DBConnect dbc;
    private static String rup_var_scenario_id;
    private static String erf_id;
    
	public static void main(String[] args) {
		if (args.length<3) {
			System.out.println("Usage: PutRuptureVariationsInDB <rup_var_scenario_id> <erf_id> < ruptVarFile | tree >");
			System.exit(0);
		}
		
        rup_var_scenario_id = args[0];
        erf_id = args[1];
        String filename = args[2];
//        dbc = new DBConnect("surface.usc.edu","CyberShake");
        
        File file = new File(filename);
		traverse(file, 0);
	}
    
    private static void traverse(File file, int i) {
        int id = i;
        if (file.isDirectory()) {
            File[] contents = file.listFiles();
            for (int j=0; j<contents.length; j++) {
                traverse(contents[j], j);
            }
        } else {
            String filename = file.getName();
            if (filename.split("\\.").length==3) { //it's a variation
                insertFile(filename, id);                
            }
        }
    }
    
    private static void insertFile(String filename, int rup_var_id) {
        String[] pieces = filename.split("\\.");
        String source_id = pieces[0].split("_")[0];
        String rupture_id = pieces[0].split("_")[1];
        
        String insertString = "insert into Rupture_Variations (Rup_Var_ID, Rup_Var_Scenario_ID, ERF_ID, Source_ID, Rupture_ID, Rup_Var_LFN) " +
            " values (" + rup_var_id + ", " + rup_var_scenario_id + ", " + erf_id + ", " + source_id + ", " + rupture_id + ", " + filename + ")";
        
        System.out.println(insertString);
    }
}