# ugem

## Table of content

- [Installation](#Installation)
- [Usage](#Usage)
- [License](#License)
- [Contributing](#Contributing)
- [TODO](#TODO)

## Installation

To build this program you will require a recent C compiler, make, `libargtable2` and `libcmocka` for unit tests.

```sh
premake gmake # to build gnu make file 
make # to compile all targets 
```

## Usage

1) Rename `ugem` to the project's name in all files and folders 

## License

This program is distributed under the terms of the MIT License.

## Contributing

## TODO

- Allow serving a directory
- Prevent any use of '..' in urls 
- Allow regex based path matching for things like mapping error codes or rewriting to another file 
- Generate a default / file that is just the index of the served directory 
- Allow rewriting that / to a file
- Option to include hidden files in the index
- Option to allow multiple root directories based on host name
