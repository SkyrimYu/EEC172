################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.obj: ../%.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"C:/TI/ccsv8/tools/compiler/ti-cgt-arm_18.1.4.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=vfplib --abi=eabi -me --include_path="C:/TI/ccsv8/tools/compiler/ti-cgt-arm_18.1.4.LTS/include" --include_path="C:/TI/CC3200SDK_1.3.0/cc3200-sdk/example/common" --include_path="C:/TI/CC3200SDK_1.3.0/cc3200-sdk/driverlib" --include_path="C:/TI/CC3200SDK_1.3.0/cc3200-sdk/inc" -g --define=cc3200 --define=ccs --diag_warning=225 --diag_wrap=off --display_error_number --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

startup_ccs.obj: C:/TI/CC3200SDK_1.3.0/cc3200-sdk/example/common/startup_ccs.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"C:/TI/ccsv8/tools/compiler/ti-cgt-arm_18.1.4.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=vfplib --abi=eabi -me --include_path="C:/TI/ccsv8/tools/compiler/ti-cgt-arm_18.1.4.LTS/include" --include_path="C:/TI/CC3200SDK_1.3.0/cc3200-sdk/example/common" --include_path="C:/TI/CC3200SDK_1.3.0/cc3200-sdk/driverlib" --include_path="C:/TI/CC3200SDK_1.3.0/cc3200-sdk/inc" -g --define=cc3200 --define=ccs --diag_warning=225 --diag_wrap=off --display_error_number --preproc_with_compile --preproc_dependency="startup_ccs.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

timer_if.obj: C:/TI/CC3200SDK_1.3.0/cc3200-sdk/example/common/timer_if.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"C:/TI/ccsv8/tools/compiler/ti-cgt-arm_18.1.4.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=vfplib --abi=eabi -me --include_path="C:/TI/ccsv8/tools/compiler/ti-cgt-arm_18.1.4.LTS/include" --include_path="C:/TI/CC3200SDK_1.3.0/cc3200-sdk/example/common" --include_path="C:/TI/CC3200SDK_1.3.0/cc3200-sdk/driverlib" --include_path="C:/TI/CC3200SDK_1.3.0/cc3200-sdk/inc" -g --define=cc3200 --define=ccs --diag_warning=225 --diag_wrap=off --display_error_number --preproc_with_compile --preproc_dependency="timer_if.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

uart_if.obj: C:/TI/CC3200SDK_1.3.0/cc3200-sdk/example/common/uart_if.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"C:/TI/ccsv8/tools/compiler/ti-cgt-arm_18.1.4.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=vfplib --abi=eabi -me --include_path="C:/TI/ccsv8/tools/compiler/ti-cgt-arm_18.1.4.LTS/include" --include_path="C:/TI/CC3200SDK_1.3.0/cc3200-sdk/example/common" --include_path="C:/TI/CC3200SDK_1.3.0/cc3200-sdk/driverlib" --include_path="C:/TI/CC3200SDK_1.3.0/cc3200-sdk/inc" -g --define=cc3200 --define=ccs --diag_warning=225 --diag_wrap=off --display_error_number --preproc_with_compile --preproc_dependency="uart_if.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

