const char key_mapping_path[] = "./data/key_mapping.db";
const char data_path[] = "./data/data.db";
const int max_key_length = 8;
const int port = 12345;
const int buf_size = 256;
const int client_num = 256;

const char connection_close_message[] = "Bye!\n";
const char error_command_message[] = "Error in input, check again.\n";
const char key_not_exist_message[] = "Key does not exist.\n";
const char done_message[] = "Done.\n";