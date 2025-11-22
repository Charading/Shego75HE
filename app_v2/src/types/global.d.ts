export type ShegoDevice = {
  pid: number;
  vid: number;
  path: string;
  product: string;
  serialNumber?: string;
};

declare global {
  interface Window {
    shego: {
      listDevices: () => Promise<ShegoDevice[]>;
      sendRaw: (pathId: string, payload: Uint8Array) => Promise<boolean>;
    };
  }
}

export {};
