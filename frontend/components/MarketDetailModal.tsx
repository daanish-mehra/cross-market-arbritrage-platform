'use client'

import { useEffect } from 'react'

interface MarketData {
  market_id: string
  market: number
  event_name: string
  best_bid: number
  best_ask: number
  bid_size: number
  ask_size: number
}

interface MarketDetailModalProps {
  market: MarketData | null
  isOpen: boolean
  onClose: () => void
}

const marketNames: { [key: number]: string } = {
  0: 'Polymarket',
  1: 'Kalshi',
  2: 'PredictIt'
}

export default function MarketDetailModal({ market, isOpen, onClose }: MarketDetailModalProps) {
  useEffect(() => {
    if (isOpen) {
      document.body.style.overflow = 'hidden'
    } else {
      document.body.style.overflow = 'unset'
    }
    return () => {
      document.body.style.overflow = 'unset'
    }
  }, [isOpen])

  if (!isOpen || !market) return null

  const midPrice = (market.best_bid + market.best_ask) / 2
  const spread = market.best_ask - market.best_bid
  const spreadPercent = midPrice > 0 ? (spread / midPrice) * 100 : 0
  const probability = midPrice > 0 ? Math.round(midPrice * 100) : 50

  return (
    <div className="modal-overlay" onClick={onClose}>
      <div className="modal-content" onClick={(e) => e.stopPropagation()}>
        <div className="modal-header">
          <h2 className="modal-title">{market.event_name}</h2>
          <button className="modal-close" onClick={onClose}>×</button>
        </div>

        <div className="modal-body">
          {/* Market Info */}
          <div className="modal-section">
            <div className="modal-info-grid">
              <div className="modal-info-item">
                <span className="modal-info-label">Market</span>
                <span className="modal-info-value">{marketNames[market.market] || `Market ${market.market}`}</span>
              </div>
              <div className="modal-info-item">
                <span className="modal-info-label">Market ID</span>
                <span className="modal-info-value font-mono">{market.market_id}</span>
              </div>
              <div className="modal-info-item">
                <span className="modal-info-label">Mid Price</span>
                <span className="modal-info-value price">{midPrice > 0 ? (midPrice * 100).toFixed(2) + '¢' : 'N/A'}</span>
              </div>
              <div className="modal-info-item">
                <span className="modal-info-label">Spread</span>
                <span className="modal-info-value">{spreadPercent.toFixed(2)}%</span>
              </div>
            </div>
          </div>

          {/* Order Book */}
          <div className="modal-section">
            <h3 className="modal-section-title">Order Book</h3>
            <div className="orderbook-grid">
              <div className="orderbook-side">
                <div className="orderbook-header">
                  <span className="orderbook-side-label bid">BID</span>
                  <span className="orderbook-header-label">Size</span>
                </div>
                {market.best_bid > 0 ? (
                  <div className="orderbook-row bid-row">
                    <span className="orderbook-price">{(market.best_bid * 100).toFixed(2)}¢</span>
                    <span className="orderbook-size">${market.bid_size.toFixed(2)}</span>
                  </div>
                ) : (
                  <div className="orderbook-row empty">No bids</div>
                )}
              </div>

              <div className="orderbook-side">
                <div className="orderbook-header">
                  <span className="orderbook-side-label ask">ASK</span>
                  <span className="orderbook-header-label">Size</span>
                </div>
                {market.best_ask > 0 ? (
                  <div className="orderbook-row ask-row">
                    <span className="orderbook-price">{(market.best_ask * 100).toFixed(2)}¢</span>
                    <span className="orderbook-size">${market.ask_size.toFixed(2)}</span>
                  </div>
                ) : (
                  <div className="orderbook-row empty">No asks</div>
                )}
              </div>
            </div>
          </div>

          {/* Probability Indicator */}
          <div className="modal-section">
            <h3 className="modal-section-title">Implied Probability</h3>
            <div className="probability-display">
              <div className="probability-value">{probability}%</div>
              <div className="probability-bar-full">
                <div 
                  className="probability-bar-fill-full buy" 
                  style={{ width: `${probability}%` }}
                />
                <div 
                  className="probability-bar-fill-full sell" 
                  style={{ width: `${100 - probability}%` }}
                />
              </div>
            </div>
          </div>

          {/* Placeholder for Chart */}
          <div className="modal-section">
            <h3 className="modal-section-title">Price History</h3>
            <div className="chart-placeholder">
              <span className="chart-placeholder-text">📈 Chart Coming Soon</span>
              <span className="chart-placeholder-subtext">Historical price data will be displayed here</span>
            </div>
          </div>
        </div>

        <div className="modal-footer">
          <button className="modal-button secondary" onClick={onClose}>Close</button>
        </div>
      </div>
    </div>
  )
}

