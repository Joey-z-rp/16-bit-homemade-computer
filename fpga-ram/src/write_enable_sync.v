module write_enable_sync #(parameter ADDR_WIDTH = 15, DATA_WIDTH = 16) (
	input wire cpu_clock,
	input wire cpu_write_enable,
	input wire [ADDR_WIDTH - 1: 0] cpu_addr,
	input wire [DATA_WIDTH - 1: 0] cpu_data,
	
	input wire ram_clock,
	output reg ram_write_enable,
	output reg [ADDR_WIDTH - 1: 0] ram_addr,
	output reg [DATA_WIDTH - 1: 0] ram_data
);
	reg cpu_write_enable_toggle = 0;
	
	always @(posedge cpu_clock) begin
		if (cpu_write_enable)
			cpu_write_enable_toggle <= ~cpu_write_enable_toggle;
	end
	
	reg sync_0 = 0;
	reg sync_1 = 0;
	reg prev_sync = 0;
	
	always @(posedge ram_clock) begin
		sync_0 <= cpu_write_enable_toggle;
		sync_1 <= sync_0;
		prev_sync <= sync_1;
		ram_write_enable <= (sync_1 != prev_sync);
		
		if (sync_1 != prev_sync) begin
			ram_addr <= cpu_addr;
			ram_data <= cpu_data;
		end
	end
endmodule
