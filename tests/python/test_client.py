from nrm import Client


def test_client_init():
    print("test_client_init")
    with Client() as nrmc:
        pass


if __name__ == "__main__":
    test_client_init()
