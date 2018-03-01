// This code checks if harward support a particular feature 
// or not. This code checks for AVX support using compiler
// intrinsic __builtin_cpu_supports.

// This code run on gcc and clang.

// Compiler: gcc cpu_feature_check.c -o cpu_feature_check
// Run: ./cpu_feature_check
 
 #include<stdio.h>

int main(){
    if(__builtin_cpu_supports("avx"))
        printf("AVX supported\n");
    else
        printf("AVX not supported\n");
return 0;
}
