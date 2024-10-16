# CSS++

**CSS++** is a CSS-like syntax parser for C++. At the moment it can only parse simple selectors
and declaration blocks like so:

```css
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
```

Any "modern" CSS features are likely not supported. The above snippet parses into a
`std::unordered_map<Selector, PropertyTable>`, where `Selector` is a string like "body" or "button" and `PropertyTable`
is a `std::unordered_map<PropertyKey, PropertyValue>`. For example, you can access the `border` property of the `button`
selector like so:

```c++
CSS::Parser parser("<css code to parse...>");
parser.parse();

if (parser.hadError) {
    parser.lastError.print();
    return -1;
}

CSS::Stylesheet stylesheet = parser.getStylesheet();
auto windowBorder = stylesheet["button"]["border"];
```

All values are stored as strings. Type conversion is up to the user, at least for now.

# License

I don't care, pick whatever one you fancy.