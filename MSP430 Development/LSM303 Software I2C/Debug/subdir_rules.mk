################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
I2C_SoftwareLibrary.obj: ../I2C_SoftwareLibrary.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/CCS/ccsv6/tools/compiler/msp430_4.3.3/bin/cl430" -vmsp --abi=eabi -Ooff --include_path="C:/CCS/ccsv6/ccs_base/msp430/include" --include_path="C:/CCS/ccsv6/tools/compiler/msp430_4.3.3/include" --advice:power=all -g --define=__MSP430G2553__ --diag_warning=225 --display_error_number --diag_wrap=off --printf_support=minimal --preproc_with_compile --preproc_dependency="I2C_SoftwareLibrary.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

dtostrf.obj: ../dtostrf.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/CCS/ccsv6/tools/compiler/msp430_4.3.3/bin/cl430" -vmsp --abi=eabi -Ooff --include_path="C:/CCS/ccsv6/ccs_base/msp430/include" --include_path="C:/CCS/ccsv6/tools/compiler/msp430_4.3.3/include" --advice:power=all -g --define=__MSP430G2553__ --diag_warning=225 --display_error_number --diag_wrap=off --printf_support=minimal --preproc_with_compile --preproc_dependency="dtostrf.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

lsm303.obj: ../lsm303.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/CCS/ccsv6/tools/compiler/msp430_4.3.3/bin/cl430" -vmsp --abi=eabi -Ooff --include_path="C:/CCS/ccsv6/ccs_base/msp430/include" --include_path="C:/CCS/ccsv6/tools/compiler/msp430_4.3.3/include" --advice:power=all -g --define=__MSP430G2553__ --diag_warning=225 --display_error_number --diag_wrap=off --printf_support=minimal --preproc_with_compile --preproc_dependency="lsm303.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

main.obj: ../main.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/CCS/ccsv6/tools/compiler/msp430_4.3.3/bin/cl430" -vmsp --abi=eabi -Ooff --include_path="C:/CCS/ccsv6/ccs_base/msp430/include" --include_path="C:/CCS/ccsv6/tools/compiler/msp430_4.3.3/include" --advice:power=all -g --define=__MSP430G2553__ --diag_warning=225 --display_error_number --diag_wrap=off --printf_support=minimal --preproc_with_compile --preproc_dependency="main.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

wiring_digital.obj: ../wiring_digital.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/CCS/ccsv6/tools/compiler/msp430_4.3.3/bin/cl430" -vmsp --abi=eabi -Ooff --include_path="C:/CCS/ccsv6/ccs_base/msp430/include" --include_path="C:/CCS/ccsv6/tools/compiler/msp430_4.3.3/include" --advice:power=all -g --define=__MSP430G2553__ --diag_warning=225 --display_error_number --diag_wrap=off --printf_support=minimal --preproc_with_compile --preproc_dependency="wiring_digital.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


