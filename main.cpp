#include <CSS.h>
#include <iostream>
#include <ranges>
#include <string>

#pragma region Test Code
const std::string testCss = R"(
/*globals {
    --background: #08090E;
}*/

window {
    background-color: #08090E;
    margin: 0;
    padding: 0;
    font-size: 14;
}

button {
    border: 1;
    border-type: solid;
    border-color: blue;
}

)";
#pragma endregion

int main() {
    CSS::Parser parser(testCss);
    parser.parse();

    if (parser.hadError) {
        parser.lastError.print();
        return -1;
    }

    CSS::Stylesheet stylesheet = parser.getStylesheet();
    CSS::PrintStylesheet(stylesheet);

    return 0;
}
