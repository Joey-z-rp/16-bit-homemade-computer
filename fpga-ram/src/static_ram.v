module static_ram #(parameter ADDR_WIDTH = 15, DATA_WIDTH = 16) (
	input wire cpu_clock,
	input wire onboard_clock,
	input wire cpu_write_enable,
	input wire [ADDR_WIDTH - 1:0] addr_in,
	input wire [DATA_WIDTH - 1:0] data_in,
	
	
	output wire [DATA_WIDTH - 1:0] data_out,
	output wire pll_locked_led,
	output wire ram_overflow
);

wire pll_locked;
wire ram_clock;
wire ram_write_enable;
wire [ADDR_WIDTH - 1:0] ram_addr;
wire [DATA_WIDTH - 1:0] ram_data;

reg [ADDR_WIDTH - 1:0] highest_addr = 0;
reg ram_overflow_reg = 1;

reg [ADDR_WIDTH - 1:0] addr_in_ram_domain;

always @(posedge cpu_clock) begin
	if (addr_in < 12289)
		highest_addr <= addr_in;
	else
		ram_overflow_reg <= 1'b0;
end

always @(posedge ram_clock) begin
	addr_in_ram_domain <= addr_in;
end

pll_50_to_100 pll (
	.inclk0(onboard_clock),
	.c0(ram_clock),
	.locked(pll_locked)
);

write_enable_sync write_enable_sync_instance (
	.cpu_clock(cpu_clock),
	.cpu_write_enable(cpu_write_enable),
	.cpu_addr(addr_in),
	.cpu_data(data_in),
	.ram_clock(ram_clock),
	.ram_write_enable(ram_write_enable),
	.ram_addr(ram_addr),
	.ram_data(ram_data)
);

m9k_ram ram_instance (
	.data(ram_data),
	.wraddress(ram_addr),
	.wren(ram_write_enable),
	.rdaddress(addr_in_ram_domain),
	.clock(ram_clock),
	.q(data_out)
);

assign ram_overflow = ram_overflow_reg;
assign pll_locked_led = ~pll_locked;

endmodule
