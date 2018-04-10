---------------------------------------------------
-- School:     University of Massachusetts Dartmouth
-- Department: Computer and Electrical Engineering
-- Engineer:   Daniel Noyes
-- 
-- Create Date:    SPRING 2015
-- Module Name:    SevenSeg
-- Project Name:   SevenSegmentDisplay
-- Target Devices: Spartan-3E
-- Tool versions:  Xilinx ISE 14.7
--
-- Description: 7-segment display controller
--  Will power the 4 7-seg displays on the Nexys 2
---------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity SSegDriver is

    port (
        CLK     : in  STD_LOGIC; -- 50 MHz input
        RST     : in  STD_LOGIC;
        SEG_0   : in STD_LOGIC_VECTOR (3 downto 0);
        SEG_1   : in STD_LOGIC_VECTOR (3 downto 0);
        SEG_2   : in STD_LOGIC_VECTOR (3 downto 0);
        SEG_3   : in STD_LOGIC_VECTOR (3 downto 0);
        SEG_4   : in STD_LOGIC_VECTOR (3 downto 0);
        SEG_5   : in STD_LOGIC_VECTOR (3 downto 0);
        SEG_6   : in STD_LOGIC_VECTOR (3 downto 0);
        SEG_7   : in STD_LOGIC_VECTOR (3 downto 0);
        DP_CTRL : in STD_LOGIC_VECTOR (7 downto 0);
        SEG_OUT : out STD_LOGIC_VECTOR (6 downto 0);
        DP_OUT  : out STD_LOGIC;
        AN_OUT  : out STD_LOGIC_VECTOR (7 downto 0)
    );

end SSegDriver;

architecture Behavioral of SSegDriver is

signal hexnum : STD_LOGIC_VECTOR (3 downto 0);
signal segnum : STD_LOGIC_VECTOR (6 downto 0);

signal clk240hz    : STD_LOGIC :='0'; -- 240Hz clock line ~= 4ms
CONSTANT wait240hz : integer := 104166; -- (50E6/240)/2 = 104166.66
signal count240hz  : integer range 0 to wait240hz := 0;

signal pos : STD_LOGIC_VECTOR (2 downto 0);

begin

SEG_OUT <= segnum;

--convert current hex to the segment display
with hexnum select
    segnum <=
        "1000000" when "0000", -- 0
        "1111001" when "0001", -- 1
        "0100100" when "0010", -- 2
        "0110000" when "0011", -- 3
        "0011001" when "0100", -- 4
        "0010010" when "0101", -- 5
        "0000010" when "0110", -- 6
        "1111000" when "0111", -- 7
        "0000000" when "1000", -- 8
        "0010000" when "1001", -- 9
        "0001000" when "1010", -- A
        "0000011" when "1011", -- B
        "1000110" when "1100", -- C
        "0100001" when "1101", -- D
        "0000110" when "1110", -- E
        "0001110" when "1111", -- F
        "1111111" when others; -- Invalid number

    clk_div_240hz: process (CLK,RST) begin
        if (RST = '1') then
            clk240hz <= '0';
            count240hz <= 0;
        elsif (rising_edge(CLK)) then
            if (count240hz = wait240hz) then
                    if(clk240hz='0') then
                        clk240hz <= '1';
                     else
                        clk240hz <= '0';
                    end if;
                count240hz <= 0;
            else
                count240hz <= count240hz + 1;
            end if;
        end if;
    end process;

    disp_driver: process (CLK) begin
    if rising_edge(clk240hz) then
            pos <= pos + 1;
            if (pos = "111") then
                pos <= "000";
            end if;
            case (pos) is
                when "000" =>
                    hexnum <= SEG_7;
                    AN_OUT <= "01111111";
                    DP_OUT <= NOT DP_CTRL(0);
                when "001" =>
                    hexnum <= SEG_6;
                    AN_OUT <= "10111111";
                    DP_OUT <= NOT DP_CTRL(1);
                when "010" =>
                    hexnum <= SEG_5;
                    AN_OUT <= "11011111";
                    DP_OUT <= NOT DP_CTRL(2);
                when "011" =>
                    hexnum <= SEG_4;
                    AN_OUT <= "11101111";
                    DP_OUT <= NOT DP_CTRL(3);
                when "100" =>
                    hexnum <= SEG_3;
                    AN_OUT <= "11110111";
                    DP_OUT <= NOT DP_CTRL(4);
                when "101" =>
                    hexnum <= SEG_2;
                    AN_OUT <= "11111011";
                    DP_OUT <= NOT DP_CTRL(5);
                when "110" =>
                    hexnum <= SEG_1;
                    AN_OUT <= "11111101";
                    DP_OUT <= NOT DP_CTRL(6);
                when "111" =>
                    hexnum <= SEG_0;
                    AN_OUT <= "11111110";
                    DP_OUT <= NOT DP_CTRL(7);
                when others =>
                    hexnum <= (others => '0');
                    AN_OUT <= (others => '0');
            end case;
        end if;
    end process;

end Behavioral;
