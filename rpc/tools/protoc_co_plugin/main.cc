/*
 * @Author: modnarshen
 * @Date: 2023.07.20 11:47:59
 * @Note: Copyrights (c) 2023 modnarshen. All rights reserved.
 */
#include "pb_co_code_generator.h"

int main(int argc, char *argv[]) {
    PbCoCodeGenerator code_generator;
    return ::pb_compiler::PluginMain(argc, argv, &code_generator);
}
