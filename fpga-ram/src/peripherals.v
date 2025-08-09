module peripherals #(parameter
	ADDR_WIDTH = 15,
	DATA_WIDTH = 16,
	KEYPRESS_DATA_WIDTH = 8
) (
	input wire cpu_clock,
	input wire cpu_write_enable,
	input wire [ADDR_WIDTH - 1: 0] cpu_addr,
	input wire [DATA_WIDTH - 1: 0] cpu_data,
	input wire [KEYPRESS_DATA_WIDTH - 1: 0] keypress_data,
	input wire [1:0] cmd_addr,
	input wire ram_clock,
	
	output wire [DATA_WIDTH - 1: 0] keypress_out_wire,
	output wire [DATA_WIDTH - 1: 0] cmd_out_wire
);
	reg [KEYPRESS_DATA_WIDTH - 1: 0] keypress_store;
	reg [KEYPRESS_DATA_WIDTH - 1: 0] keypress_out;
	
	always @(posedge ram_clock) begin
		keypress_store <= keypress_data;
		keypress_out <= keypress_store;
	end
	
	assign keypress_out_wire = {8'b0, keypress_out};
	
	
	reg [DATA_WIDTH - 1: 0] cmd_regs [3:0];
	reg [DATA_WIDTH - 1: 0] cmd_sync [3:0];
	reg [DATA_WIDTH - 1: 0] cmd_out;
	
	always @(posedge cpu_clock) begin
		if (cpu_write_enable && cpu_addr[ADDR_WIDTH - 1:2] == 13'b1000000000000) begin
			case (cpu_addr[1:0])
				3'd0: cmd_regs[0] <= cpu_data;
				3'd1: cmd_regs[1] <= cpu_data;
				3'd2: cmd_regs[2] <= cpu_data;
				3'd3: cmd_regs[3] <= cpu_data;
				default: ; // Do nothing
			endcase
		end
	end
	
	always @(posedge ram_clock) begin
		cmd_sync[0] <= cmd_regs[0];
		cmd_sync[1] <= cmd_regs[1];
		cmd_sync[2] <= cmd_regs[2];
		cmd_sync[3] <= cmd_regs[3];
		
		case (cmd_addr)
			3'd0: cmd_out <= cmd_sync[0];
			3'd1: cmd_out <= cmd_sync[1];
			3'd2: cmd_out <= cmd_sync[2];
			3'd3: cmd_out <= cmd_sync[3];
			default: ; // Do nothing
		endcase
	end
	
	assign cmd_out_wire = cmd_out;
	
endmodule
