LIBRARY IEEE;
USE IEEE.std_logic_1164.all;
ENTITY sig_generator IS
	PORT( 
	      inst                  :   IN std_logic_vector (18 DOWNTO 0);
 	      ir                    :   IN std_logic_vector(15 DOWNTO 0 );
 	      MPC                   :   IN std_logic_vector(4 DOWNTO 0);
 	      
	     	alu                   :   out std_logic_vector(7 DOWNTO 0);
	     	alu1                  :   out std_logic_vector (10 DOWNTO 0);
	     	alu2                  :   out std_logic_vector(15 DOWNTO 0);
	     	out1                  :   out std_logic_vector (7 DOWNTO 0);
	     	out2                  :   out std_logic_vector (7 DOWNTO 0);
	     	in1                   :   out std_logic_vector (7 DOWNTO 0);
	     	in2                   :   out std_logic_vector (3 DOWNTO 0); 
	     	rout                  :   out std_logic_vector (7 DOWNTO 0);
	     	rin                   :   out std_logic_vector (7 DOWNTO 0)
	     	);
END ENTITY sig_generator;


ARCHITECTURE arch1 of sig_generator IS
  
	component mux is 
    GENERIC ( n : INTEGER := 3 ) ;
    PORT( grp : in std_logic_vector(n-1 downto 0);
	      signals : out std_logic_vector (  (2**n)-1 downto 0)
	     	 );
	  end component;
	 
	function reduction ( input    : in std_logic_vector(3 downto 0))
    return std_logic is
    variable res : std_logic := '1';
  begin
    for i in 0 to 7 loop
      res := res and input(i);
    end loop;
    return res;
     
end function reduction;
	 
	 
  signal temp,temp1,temp2,temp6 : std_logic_vector(7 downto 0);
  signal temp3,temp4,temp5, temp8 : std_logic ;
  signal alu1_temp : std_logic_vector(31 downto 0);
  signal alu2_temp : std_logic_vector(15 downto 0);
  signal temp_out1 ,  temp_in1 : std_logic_vector(7 downto 0); 
  
 

BEGIN
  
  f0: mux GENERIC MAP (n=>3) PORT MAP(inst(13 DOWNTO 11) , temp6);
  f1: mux GENERIC MAP (n=>3) PORT MAP(inst(10 DOWNTO 8) , temp_out1);
  f2: mux GENERIC MAP (n=>3) PORT MAP(inst(7 DOWNTO 5) , out2);
  f3: mux GENERIC MAP (n=>3) PORT MAP(inst(4 DOWNTO 2) , temp_in1);
  f4: mux GENERIC MAP (n=>2) PORT MAP(inst(1 DOWNTO 0) , in2);
  f5: mux GENERIC MAP (n=>3) PORT MAP(ir(8 downto 6) , temp);
  f6: mux GENERIC MAP (n=>3) PORT MAP(ir(2 downto 0) , temp2);
  
  f7 : mux GENERIC MAP (n=>4) PORT MAP(IR(15 DOWNTO 12) , alu2_temp); -- 2operands
  f8 : mux generic map(n=>5) PORT MAP(IR(10 DOWNTO 6) , alu1_temp); -- 1 operand
    
  alu2_temp(0) <= alu2_temp(0) or alu2_temp(13); -- nop A at mov and cmp
    
  out1 <= temp_out1;
  in1 <= temp_in1;
  
  
    -- decode all groups
   
   rout <= temp  when  temp_out1(0) = '1' else
           temp2 when  temp_out1(1) = '1' else 
            "00000000";
   temp1 <=  temp when temp_in1(3) = '1' else
           temp2 when temp_in1(4) = '1' else 
           "00000000";
       
   temp3 <= reduction(ir(15 DOWNTO 12) and "1110"); 
   temp4 <= reduction(mpc(3 downto 0) xnor "1001") and mpc(4);
   temp5 <= temp3 nand temp4;
   
   -- temp7 <= reduction(mpc xnor "11101");
   temp8 <= temp3 nand temp5;
    
   rin <= temp1 when temp5 = '1' else 
           "00000000";   
   
   -- case mdrin      
   alu <= temp6 when temp8 = '1' else 
          "00000000";
    
   alu2 <= alu2_temp when  ((inst(18 downto 14) = "11001") or  (inst(18 downto 14) = "11101") )  and ((ir(15) or ir(14) or ir(13) or ir(12)) = '1') else
          "0000000000000000";
   
   alu1 <= alu1_temp(10 downto 0) when ((inst(18 downto 14) = "11001") or (inst(18 downto 14) = "11101" ))and ((ir(15) or ir(14) or ir(13) or ir(12)) = '0') else
          "00000000000";   

END arch1;  

