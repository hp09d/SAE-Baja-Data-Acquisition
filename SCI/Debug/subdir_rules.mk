################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
main.obj: ../main.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"c:/ti/ccsv6/tools/compiler/arm_5.1.6/bin/armcl" -mv7R4 --code_state=32 --abi=eabi --include_path="c:/ti/ccsv6/tools/compiler/arm_5.1.6/include" --include_path="C:/Users/Hebe/workspace_v6_0/SCI/include" -g --display_error_number --diag_warning=225 --diag_wrap=off --enum_type=packed --preproc_with_compile --preproc_dependency="main.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


